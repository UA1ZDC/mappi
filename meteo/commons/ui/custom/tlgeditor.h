#ifndef METEO_COMMONS_UI_CUSTOM_TLGEDITOR_H
#define METEO_COMMONS_UI_CUSTOM_TLGEDITOR_H

#include <qwidget.h>

namespace Ui {
class TlgEditor;
}

class QTextEdit;

//!
class TlgEditor : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
  Q_PROPERTY(Format tlgFormat READ tlgFormat WRITE setTlgFormat)

  Q_ENUMS(Format)

public:
  //!
  enum Format { kText, kBinary };

  //!
  explicit TlgEditor(QWidget* parent = nullptr );
  //!
  virtual ~TlgEditor();

  //!
  void setCodecName(const QString& name) { codecName_ = name; }
  //!
  void setStartLine(const QByteArray& start);
  //!
  void setHeader(const QByteArray& head);
  //!
  void setMessage(const QByteArray& msg, Format format);
  //!
  void setEnd(const QByteArray& end);

  //!
  QString codecName() const { return codecName_; }
  //!
  QByteArray startLine() const;
  //!
  QByteArray header() const;
  //!
  QByteArray message() const;
  //!
  QByteArray end() const;
  //!
  QByteArray tlg() const;

  //!
  Format tlgFormat() const { return format_; }
  //!
  void setTlgFormat(Format format);
  //!
  bool isReadOnly() const { return readOnly_; }
  //!
  void setReadOnly(bool ro);

  //!
  void insertData(const QByteArray& data);

  //!
  void clear();
  //!
  void setModified(bool m);
  //!
  bool isModified() const;

private:
  void insertData(QTextEdit* editor, const QByteArray& data) const;
  void setTlgData(QTextEdit* editor, const QByteArray& data) const;
  QByteArray getTlgData(QTextEdit* editor) const;

private:
  // параметры, свойства
  bool readOnly_;
  Format format_;

  // данные
  QString codecName_;
  QByteArray message_;

  // служебные
  Ui::TlgEditor* ui_;
};

#endif // METEO_COMMONS_UI_CUSTOM_TLGEDITOR_H
