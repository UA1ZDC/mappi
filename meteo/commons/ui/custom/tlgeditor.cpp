#include "tlgeditor.h"
#include "ui_tlgeditor.h"

#include <cross-commons/debug/tlog.h>
#include <qtextcodec.h>
#include <qtextobject.h>

#include <commons/funcs/tlgtextobject.h>


TlgEditor::TlgEditor(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::TlgEditor)
{
  ui_->setupUi(this);

  readOnly_ = false;
  format_ = kText;
  codecName_ = "UTF-8";

  QObject *tlgInterface = new TlgTextObject;
  ui_->editor->document()->documentLayout()->registerHandler(TlgTextFormat, tlgInterface);
  ui_->endEditor->document()->documentLayout()->registerHandler(TlgTextFormat, tlgInterface);
  ui_->startEditor->document()->documentLayout()->registerHandler(TlgTextFormat, tlgInterface);
  ui_->headEditor->document()->documentLayout()->registerHandler(TlgTextFormat, tlgInterface);
}

TlgEditor::~TlgEditor()
{
  delete ui_;
}

void TlgEditor::setStartLine(const QByteArray& start)
{
  setTlgData(ui_->startEditor, start);
}

void TlgEditor::setHeader(const QByteArray& head)
{
  setTlgData(ui_->headEditor, head);
}

void TlgEditor::setMessage(const QByteArray& msg, Format format)
{
  message_ = msg;

  if ( kText == format  ) {
    setTlgData(ui_->editor, msg);
  }
  else {
    ui_->editor->setPlainText(tr("< бинарные данные >\n"));
  }
}

void TlgEditor::setEnd(const QByteArray& end)
{
  setTlgData(ui_->endEditor, end);
}

QByteArray TlgEditor::startLine() const
{
  return getTlgData(ui_->startEditor);
}

QByteArray TlgEditor::header() const
{
  return getTlgData(ui_->headEditor);
}

QByteArray TlgEditor::message() const
{
  if ( kBinary == format_ ) { return message_; }
  if ( isReadOnly() ) { return message_; }

  return getTlgData(ui_->editor);
}

QByteArray TlgEditor::end() const
{
  return getTlgData(ui_->endEditor);
}

QByteArray TlgEditor::tlg() const
{
  return startLine() + header() + message() + end();
}

void TlgEditor::setTlgFormat(TlgEditor::Format format)
{
  format_ = format;

  // для обновления виджета
  setReadOnly(isReadOnly());

  bool isBinFormat = ( kBinary == format_ );

  ui_->editor->setReadOnly(isBinFormat);
}

void TlgEditor::setReadOnly(bool ro)
{
  QTextCodec* codec = QTextCodec::codecForName(codecName_.isEmpty() ? "UTF-8" : codecName_.toUtf8());

  if ( 0 == codec ) { return; }

  if ( ro ) {
    QByteArray mes = ( kText == format_ ) ? message() : codec->fromUnicode(tr("< бинарные данные >\n"));
    setTlgData(ui_->editor, startLine() + header() + mes + end());
  }
  else {
    QByteArray s = startLine();
    QByteArray h = header();
    QByteArray e = end();
    QByteArray m = message();

    clear();

    setStartLine(s);
    setHeader(h);
    setEnd(e);
    setMessage(m, format_);
  }

  readOnly_ = ro;

  ui_->editor->setReadOnly(readOnly_);

  ui_->startEditor->setVisible(!readOnly_);
  ui_->endEditor->setVisible(!readOnly_);
  ui_->headEditor->setVisible(!readOnly_);
}

void TlgEditor::insertData(const QByteArray& data)
{
  if ( ui_->startEditor->hasFocus() ) {
    insertData(ui_->startEditor, data);
  }
  else if ( ui_->headEditor->hasFocus() ) {
    insertData(ui_->headEditor, data);
  }
  else if ( ui_->editor->hasFocus() ) {
    insertData(ui_->editor, data);
  }
  else if ( ui_->endEditor->hasFocus() ) {
    insertData(ui_->endEditor, data);
  }
}

void TlgEditor::clear()
{
  ui_->startEditor->clear();
  ui_->headEditor->clear();
  ui_->editor->clear();
  ui_->endEditor->clear();
}

void TlgEditor::setModified(bool m)
{
  ui_->startEditor->document()->setModified(m);
  ui_->headEditor->document()->setModified(m);
  ui_->editor->document()->setModified(m);
  ui_->endEditor->document()->setModified(m);
}

bool TlgEditor::isModified() const
{
  return  ui_->startEditor->document()->isModified()
          ||  ui_->headEditor->document()->isModified()
          ||  ui_->editor->document()->isModified()
          ||  ui_->endEditor->document()->isModified();
}

void TlgEditor::insertData(QTextEdit* editor, const QByteArray& data) const
{
  QTextCodec* codec = QTextCodec::codecForName(codecName_.isEmpty() ? "UTF-8" : codecName_.toUtf8());

  if ( 0 == codec ) { return; }

  QString msg = codec->toUnicode(data);

  for ( int i=0,isz=msg.size(); i<isz; ++i ) {
    QTextCursor cursor = editor->textCursor();

    QChar c = msg.at(i);
    if ( TlgTextObject::isSpecChar(c) ) {
      QTextCharFormat tlgCharFormat;
      tlgCharFormat.setObjectType(TlgTextFormat);

      tlgCharFormat.setProperty(TlgTextObject::Symbol, TlgTextObject::kSpecChars.key(c));
      tlgCharFormat.setProperty(TlgTextObject::Value, c);
      tlgCharFormat.setProperty(TlgTextObject::Font, editor->font());

      cursor.insertText(QString(QChar::ObjectReplacementCharacter), tlgCharFormat);

      if ( QChar(012) == c ) { cursor.insertBlock(); } // переход на новую строку после '\n'
    }
    else {
      cursor.insertText(c);
    }
  }
}

void TlgEditor::setTlgData(QTextEdit* editor, const QByteArray& data) const
{
  editor->clear();
  insertData(editor, data);
  editor->document()->setModified(false);
}

QByteArray TlgEditor::getTlgData(QTextEdit* editor) const
{
  QTextCodec* codec = QTextCodec::codecForName(codecName_.isEmpty() ? "UTF-8" : codecName_.toUtf8());

  if ( 0 == codec ) { return QByteArray(); }

  QByteArray raw;

  QTextDocument* doc = editor->document();
  QTextBlock block = doc->begin();
  while ( block.isValid() ) {
    QTextBlock::iterator it;
    for (it = block.begin(); !(it.atEnd()); ++it) {
      QTextFragment fragment = it.fragment();
      if (fragment.isValid() ) {
        if ( TlgTextFormat == fragment.charFormat().objectType() ) {
          raw += QByteArray(fragment.length(), fragment.charFormat().property(TlgTextObject::Value).toChar().toLatin1());
        }
        else {
          raw += codec->fromUnicode(fragment.text());
        }
      }
    }

    block = block.next();
  }

  return raw;
}
