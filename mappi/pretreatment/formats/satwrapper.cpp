#include "satwrapper.h"

namespace mappi {
  namespace po {
    namespace singleton {
      template<> mappi::po::SatDump::Wrapper *SatDumpWrapper::_instance = 0;
    }
  }
}

namespace mappi::po::SatDump {
  Wrapper::Wrapper(QString weather_file, slog::LogLevel logLevel, bool thematic_toggle) {
    initLogger();
    logger->set_level(logLevel);

    // Init SatDump
    satdump::tle_file_override = weather_file.toStdString();
    logger->set_level(slog::LOG_ERROR);
    satdump::initSatdump();
    satdump::config::main_cfg["satdump_general"]["auto_process_products"]["value"] = thematic_toggle;
    logger->set_level(logLevel);
  }

  bool Wrapper::pipelineHasCadu(const QString pipelineName){
    std::string pipeline_name = pipelineName.toStdString();
    std::optional<satdump::Pipeline> pipeline_opt = satdump::getPipelineFromName(pipeline_name);
    if (!pipeline_opt.has_value()) return false;

    for (auto step : pipeline_opt.value().steps) {
      if (step.level_name == "cadu") return true;
    }
    return false;
  }

  std::optional<satdump::Pipeline> Wrapper::setupPipeline(Params &params){
    std::string pipeline_name = params.pipeline_name.toStdString();
    std::string output_dir = params.output_dir.toStdString();

    // Create output dir
    if (!std::filesystem::exists(output_dir)) std::filesystem::create_directories(output_dir);
    // Get pipeline
    return satdump::getPipelineFromName(pipeline_name);
  }

  bool Wrapper::runPipeline(std::optional<satdump::Pipeline> &pipeline_opt, const Params &params) {
    if (pipeline_opt.has_value()) {
      return runPipeline(pipeline_opt.value(), params);
    } else {
      logger->error("Pipeline has no value");
      return false;
    }
  }

  bool Wrapper::runPipeline(satdump::Pipeline &pipeline, const Params &params) {
    std::string input_file = params.input_file.toStdString();
    std::string input_level = params.input_level.toStdString();
    std::string output_dir = params.output_dir.toStdString();
    nlohmann::json parameters = nlohmann::json::parse(params.additional_params.toStdString());

    if (std::filesystem::exists(input_file) && !std::filesystem::is_directory(input_file)) {
      HeaderInfo hdr = try_parse_header(input_file);
      if (hdr.valid) {
        parameters["samplerate"] = hdr.samplerate;
        parameters["baseband_format"] = hdr.type;
      }
    }

    try {
      logger->debug("Running pipeline: " + pipeline.readable_name + "(" + pipeline.name + ")");
      pipeline.run(input_file, output_dir, parameters, input_level);
    }
    catch (std::exception &e) {
      logger->error("Fatal error running pipeline : " + std::string(e.what()));
      return false;
    }
    return true;
  }

  QMap<QString, Product> Wrapper::getProducts(const QString &output_dir) const {
    logger->trace("Getting products for" + output_dir.toStdString());
    if (!QFile::exists(output_dir + "/dataset.json")) return getProductsFallback(output_dir);

    satdump::ProductDataSet dataset;
    dataset.load(output_dir.toStdString() + "/dataset.json");
    QDateTime roughDateTime = QDateTime::fromMSecsSinceEpoch(dataset.timestamp * 1000);

    QMap<QString, Product> products;
    for (std::string pName : dataset.products_list) {
      QString productName = QString::fromStdString(pName);
      QString productPath = QString("%1/%2/product.cbor").arg(output_dir, productName);

      QVector<ImageProduct> images;
      QVector<double> timestamps;

      //TODO: remove in production
      writeJSON(readCBOR(productPath), QString("%1/%2/product.json").arg(output_dir, productName));

      int bit_depth = 16;
      nlohmann::ordered_json projection_cfg;
      satdump::Products raw_products;
      raw_products.load(productPath.toStdString());
      if(raw_products.type == "image"){
        satdump::ImageProducts img_products;
        img_products.load(productPath.toStdString());
        auto img_timestamps = img_products.get_timestamps(-1);
        timestamps = QVector<double>(img_timestamps.begin(), img_timestamps.end());
        bit_depth = img_products.bit_depth;
        if(img_products.has_proj_cfg()) projection_cfg = img_products.contents["projection_cfg"];

        for (auto image : img_products.images) {
          images.push_back({
            QString("%1/%2/%3").arg(output_dir, productName, QString::fromStdString(image.filename)),
            image.image,
            QString::fromStdString(image.channel_name),
            image.ifov_y,
            image.ifov_x,
            image.offset_x
          });
        }
      }else if(raw_products.type == "radiation"){
        satdump::RadiationProducts rad_products;
        rad_products.load(productPath.toStdString());
        auto rad_timestamps = rad_products.get_timestamps(-1);
        timestamps = QVector<double>(rad_timestamps.begin(), rad_timestamps.end());
        //TODO: может сохранять
      }else if(raw_products.type == "scatterometer"){
        satdump::ScatterometerProducts rad_products;
        rad_products.load(productPath.toStdString());
        auto rad_timestamps = rad_products.get_timestamps(-1);
        timestamps = QVector<double>(rad_timestamps.begin(), rad_timestamps.end());
        if(rad_products.has_proj_cfg()) projection_cfg = rad_products.contents["projection_cfg"];
        //TODO: может сохранять
      }

      auto getMedian = [](QVector<double>& sorted_vec){
        int vec_size = sorted_vec.size();
        if(vec_size % 2)
          return sorted_vec.at(vec_size / 2);
        else
          return (sorted_vec.at(vec_size / 2 - 1) + sorted_vec.at(vec_size / 2)) * .5;
      };

      QDateTime dtStart, dtEnd;
      if(timestamps.size() > 0) {
        logger->debug("Timestamps size: " + std::to_string(timestamps.size()));

        std::sort(timestamps.begin(), timestamps.end());
        double timestampsMedian = getMedian(timestamps);
        logger->debug("Timestamps median: " + std::to_string(timestampsMedian));

        QVector<double> difference = QVector<double>(timestamps);
        for (double &ts: difference) ts = std::abs(ts - timestampsMedian);

        QVector<double> sortedDifference = QVector<double>(difference);
        std::sort(sortedDifference.begin(), sortedDifference.end());
        double medianAbsDeviation = getMedian(sortedDifference);
        logger->debug("Median absolute deviation: " + std::to_string(medianAbsDeviation));
        double threshold = medianAbsDeviation * 3;

        int ts_count = timestamps.size();
        int skipStart = 0;
        while (difference.at(skipStart) > threshold && skipStart < ts_count) skipStart++;
        dtStart = QDateTime::fromMSecsSinceEpoch(timestamps.at(skipStart) * 1000);
        int skipEnd = 0;
        while (difference.at(ts_count - 1 - skipEnd) > threshold && skipEnd < ts_count) skipEnd++;
        dtEnd = QDateTime::fromMSecsSinceEpoch(timestamps.at(ts_count - 1 - skipEnd) * 1000);
        logger->debug("Removed: " + std::to_string(skipStart + skipEnd) + " timestamps");
      }

      if(!dtStart.isValid() || !dtEnd.isValid()){
        logger->warn("Got invalid timestamps. Using rough timestamp from dataset");
        dtStart = roughDateTime;
        dtEnd = dtStart;
      }

      const short MAX_DAYS_BETWEEN_DATES = 1;
      if(std::abs(dtStart.daysTo(roughDateTime)) > MAX_DAYS_BETWEEN_DATES){
        logger->warn("dtStart(" + dtStart.toString("dd.MM.yyyy hh:mm").toStdString() + ") is too far. Taking only time");
        QTime startTime = dtStart.time();
        dtStart = roughDateTime;
        dtStart.setTime(startTime);
      }
      if(std::abs(dtEnd.daysTo(roughDateTime)) > MAX_DAYS_BETWEEN_DATES){
        logger->warn("dtEnd(" + dtEnd.toString("dd.MM.yyyy hh:mm").toStdString() + ") is too far. Taking only time");
        QTime endTime = dtEnd.time();
        dtEnd = roughDateTime;
        dtEnd.setTime(endTime);
      }

      QString dateString = dtStart.toString("dd.MM.yyyy hh:mm") + " -> " + dtEnd.toString("dd.MM.yyyy hh:mm");
      logger->debug("Product has " + std::to_string(images.size()) + " images. Date: " + dateString.toStdString());
      Product curProduct = {images, qMakePair(dtStart, dtEnd), bit_depth, projection_cfg};
      products.insert(productName, curProduct);
    }
    return products;
  }

  QMap<QString, Product> Wrapper::getProductsFallback(const QString &output_dir) const {
    logger->warn("Using fallback method to get products. Cannot find dataset information");
    QDirIterator subdirs(output_dir, QDir::AllDirs, QDirIterator::NoIteratorFlags);
    QMap<QString, Product> products;

    while (subdirs.hasNext()) {
      QString productName = subdirs.next();
      QDirIterator png_files(QString("%1/%2").arg(output_dir, productName), {"*.png", "*.PNG"}, QDir::Files);

      QVector<ImageProduct> images;
      image::Image<uint16_t> png_image;
      while (png_files.hasNext()) {
        QString file = png_files.next();
        QString filePath = QString("%1/%2/%3").arg(output_dir,productName, file);
        png_image.load_png(filePath.toStdString());
        images.push_back({ filePath, png_image });
      }

      nlohmann::ordered_json projection_cfg;
      Product curProduct = {images, qMakePair(QDateTime(), QDateTime())};
      products.insert(productName, curProduct);
    }
    return products;
  }

  nlohmann::json Wrapper::readJSON(const QString &fileNameIn) const {
    std::ifstream f(fileNameIn.toStdString());
    return nlohmann::json::parse(f);
  }

  void Wrapper::writeJSON(const nlohmann::json &contents, const QString &fileNameOut) const {
    std::ofstream output_file(fileNameOut.toStdString(), std::ios::binary);
    output_file << contents.dump(4);
    output_file.close();
  }

  nlohmann::json Wrapper::readCBOR(const QString &fileNameIn) const {
    std::vector<uint8_t> cbor_data;
    std::ifstream in_file(fileNameIn.toStdString(), std::ios::binary);
    while (!in_file.eof()) {
      uint8_t b;
      in_file.read((char *) &b, 1);
      cbor_data.push_back(b);
    }
    in_file.close();
    cbor_data.pop_back();
    return nlohmann::json::from_cbor(cbor_data);
  }
}