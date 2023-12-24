#ifndef SATDUMP_WRAPPER_INIT_H
#define SATDUMP_WRAPPER_INIT_H

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QMap>

#include <sat-commons/satellite/satviewpoint.h>
#include <mappi/global/streamheader.h>
#include <mappi/pretreatment/formats/satformat.h>

#undef none_msg
#undef debug_msg
#undef info_msg
#undef warning_msg
#undef error_msg
#undef critical_msg

#include "common/detect_header.h"
#include "init.h"
#include "common/cli_utils.h"
#include "core/module.h"
#include "core/pipeline.h"
#include "core/config.h"
#include "products/dataset.h"
#include "products/products.h"
#include "products/image_products.h"
#include "products/radiation_products.h"
#include "products/scatterometer_products.h"
#include "common/image/image.h"
#include <filesystem>
#include "nlohmann/json.hpp"
#include "logger.h"

#define DEFAULT_LOG_LEVEL slog::LOG_INFO

namespace mappi::po::SatDump {
  struct Params {
    QString input_file;
    QString input_level;
    QString pipeline_name;
    QString output_dir;
    QString additional_params;
  };

  struct ImageProduct {
    QString file_path;
    image::Image<uint16_t> image;
    QString channel_name = "";
    int ifov_y = -1;
    int ifov_x = -1;
    int offset_x = 0;
  };

  struct ImageComposite {
    QString file_path;
    QString instrument_name;
    QString composite_name;
    QString thematic_name;
  };

  struct Product{
    QVector<ImageProduct> images;
    QPair<QDateTime, QDateTime> time;
    int bit_depth = 16;
    nlohmann::ordered_json projection_cfg = nlohmann::ordered_json();
  };

  template<typename T>
  using Image = image::Image<T>;

  class Wrapper {
  public:
    static Wrapper & getInstance(){
      static Wrapper wrapper;
      return wrapper;
    }

    Wrapper(const QString& weather_file,
            const slog::LogLevel logLevel = DEFAULT_LOG_LEVEL,
            const bool thematic_enabled = true
    );

    Wrapper(){
      Wrapper(
          singleton::SatFormat::instance()->getWeatherFilePath()
      );
    }

    bool pipelineHasCadu(const QString pipeline_name);

    std::optional<satdump::Pipeline> setupPipeline(Params &params);
    bool runPipeline(std::optional<satdump::Pipeline> &pipeline_opt, const Params &params);
    bool runPipeline(satdump::Pipeline &pipeline_opt, const Params &params);
    QMap<QString, Product> getProducts(const QString &output_dir) const;
    QMap<QString, Product> getProductsFallback(const QString &output_dir) const;
    QVector<ImageComposite> getComposites(const QString &output_dir) const;

    nlohmann::json readJSON(const QString &fileNameIn) const;
    nlohmann::json readCBOR(const QString &fileNameIn) const;
    void writeJSON(const nlohmann::json &contents, const QString &fileNameOut) const;
  };
}
#endif //SATDUMP_WRAPPER_INIT_H
