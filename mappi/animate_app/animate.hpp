#include "gif.h"
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPair>

#define DEFAULT_DELAY 1000

namespace mappi::Animate{
  class GifBuilder{
  public:
    GifBuilder(const QString& path): output_path(path){}

    void setWidth(const int new_width){ size.setWidth(new_width); };
    void setHeight(const int new_height){ size.setHeight(new_height); };
    void setSize(const QSize& new_size){ size = new_size; };
    void setOutput(const QString& path){ output_path = path; }
    void setDelay(const int new_delay){ delay = new_delay; }
    void setBitDepth(const int new_depth){ bitDepth = new_depth; }
    void setDither(const bool new_dither){ dither = new_dither; }

    bool updateFrame(const int idx, const QString& new_path){
      if(idx >= image_files.size()) return false;
      image_files[idx] = new_path;
      return true;
    }
    bool updateFrameDelay(const int idx, const int new_delay){
      if(idx >= image_delays.size()) return false;
      if(new_delay < 0) return false;
      image_delays[idx] = new_delay /10;
      return true;
    }

    int getWidth() const { return size.width(); };
    int getHeight() const { return size.height(); };
    QSize getSize() const { return size; };
    QString getOutput() const { return output_path; }
    unsigned int getDelay() const { return delay; }
    int getBitDepth() const { return bitDepth; }
    bool getDither() const { return dither; }
    int getFrameCount() const {return std::min(image_files.size(), image_delays.size()); }
    QString getFrame(const int idx) const {
      if(idx >= image_files.size()) return QString();
      return image_files.at(idx);
    }
    unsigned int getFrameDelay(const int idx) const {
      if(idx >= image_delays.size()) return false;
      return image_delays.at(idx) * 10;
    }

    bool addFrame(const QString& image_path, unsigned int image_delay = DEFAULT_DELAY){
      reader.setFileName(image_path);
      if(!reader.canRead()) return false;

      image_files.push_back(image_path);
      if (reader.size().width()  > size.width())  setWidth (reader.size().width());
      if (reader.size().height() > size.height()) setHeight(reader.size().height());

      image_delays.push_back(image_delay / 10);
      return true;
    }

    void writeGif(){
      const char* filename = output_path.toLocal8Bit().data();
      QImage frame(size, QImage::Format_RGBA8888);
      GifWriter writer = {};
      GifBegin( &writer, filename, size.width(), size.height(), delay / 10, bitDepth, dither );

      for(int i = 0; i < getFrameCount(); i++){
        reader.setFileName(image_files[i]);
        QSize offset = (size - reader.size()) / 2;
        QPoint destPos = QPoint(offset.width(), offset.height());

        frame.fill(Qt::white);
        QPainter painter(&frame);
        painter.drawImage(destPos, reader.read());
        painter.end();

        GifWriteFrame( &writer, frame.constBits(), size.width(), size.height(), image_delays[i], bitDepth, dither );
      }

      GifEnd( &writer );
    }

  private:
    unsigned int delay = DEFAULT_DELAY; //delay in ms
    QImageReader reader;
    QString output_path;
    QVector<QString> image_files;
    QVector<int> image_delays;
    QSize size;
    int32_t bitDepth = 8;
    bool dither = false;
  };
}