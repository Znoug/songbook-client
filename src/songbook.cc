// Copyright (C) 2010 Romain Goffe, Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************
#include "songbook.hh"

#include <QFile>
#include <QTextStream>

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QToolButton>
#include <QColor>
#include <QColorDialog>
#include <QFileDialog>
#include <QDir>

#include <QScriptEngine>

#include <QDebug>

CSongbook::CSongbook()
  : QObject()
  , m_title()
  , m_subtitle()
  , m_author()
  , m_version()
  , m_mail()
  , m_picture()
  , m_pictureCopyright()
  , m_footer()
  , m_licence()
  , m_shadeColor()
  , m_fontSize()
  , m_tmpl()
  , m_bookType()
  , m_songs()
  , m_panel()
{}

CSongbook::~CSongbook()
{
  if (m_panel)
    delete m_panel;
}

QString CSongbook::title()
{
  return m_title;
}
void CSongbook::setTitle(const QString &title)
{
  m_title = title;
}

QString CSongbook::subtitle()
{
  return m_subtitle;
}
void CSongbook::setSubtitle(const QString &subtitle)
{
  m_subtitle = subtitle;
}

QString CSongbook::author()
{
  return m_author;
}
void CSongbook::setAuthor(const QString &author)
{
  m_author = author;
}

QString CSongbook::version()
{
  return m_version;
}
void CSongbook::setVersion(const QString &version)
{
  m_version = version;
}

QString CSongbook::mail()
{
  return m_mail;
}
void CSongbook::setMail(const QString &mail)
{
  m_mail = mail;
}

QString CSongbook::picture()
{
  return m_picture;
}
void CSongbook::setPicture(const QString &picture)
{
  m_picture = picture;
}

QString CSongbook::pictureCopyright()
{
  return m_pictureCopyright;
}
void CSongbook::setPictureCopyright(const QString &pictureCopyright)
{
  m_pictureCopyright = pictureCopyright;
}

QString CSongbook::footer()
{
  return m_footer;
}
void CSongbook::setFooter(const QString &footer)
{
  m_footer = footer;
}

QString CSongbook::licence()
{
  return m_licence;
}
void CSongbook::setLicence(const QString &licence)
{
  m_licence = licence;
}

QString CSongbook::shadeColor()
{
  return m_shadeColor;
}
void CSongbook::setShadeColor(const QString &shadeColor)
{
  m_shadeColor = shadeColor;
}

QString CSongbook::fontSize()
{
  return m_fontSize;
}
void CSongbook::setFontSize(const QString &fontSize)
{
  m_fontSize = fontSize;
}

QString CSongbook::tmpl()
{
  return m_tmpl;
}
void CSongbook::setTmpl(const QString &tmpl)
{
  m_tmpl = tmpl;
}

QStringList CSongbook::bookType()
{
  return m_bookType;
}
void CSongbook::setBookType(QStringList bookType)
{
  m_bookType = bookType;
}

QStringList CSongbook::songs()
{
  return m_songs;
}

void CSongbook::setSongs(QStringList songs)
{
  m_songs = songs;
}

void CSongbook::save(const QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream out(&file);
      out << "{\n";

      if (!tmpl().isEmpty())
        out << "\"template\" : \"" << tmpl() << "\",\n";

      QStringList properties;
      QString property;
      QString value;
      properties << "title" << "author" << "subtitle" << "mail" 
                 << "version" << "picture" << "footer" << "licence"
                 << "pictureCopyright" << "shadeColor" << "fontSize";
      foreach (property, properties)
        {
          value = QObject::property(property.toStdString().c_str()).toString();
          if (!value.isEmpty())
            {
              out << "\"" << property.toLower() 
                  << "\" : \"" 
                  << value << "\",\n";
            }
        }

      out << "\"songs\" : [\n    \"" << (songs().join("\",\n    \"")) << "\"\n  ]\n}\n";
      file.close();
    }
  else
    {
      qWarning() << "unable to open file in write mode";
    }
}

void CSongbook::load(const QString & filename)
{
  QFile file(filename);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString json = QString("(%1)").arg(in.readAll());
      file.close();

      // Load json encoded songbook data
      QScriptValue object;
      QScriptEngine engine;

      // check syntax
      QScriptSyntaxCheckResult res = QScriptEngine::checkSyntax(json);
      if (res.state() != QScriptSyntaxCheckResult::Valid)
        {
          qDebug() << "Error line "<< res.errorLineNumber()
                   << " column " << res.errorColumnNumber()
                   << ":" << res.errorMessage();
        }
      // evaluate the json data
      object = engine.evaluate(json);

      // load data into this object
      if (object.isObject())
        {
          QScriptValue sv;
          // template property
          sv = object.property("template");
          if (sv.isValid())
            setTmpl(sv.toString());

          // default property
          QStringList properties;
          QString property;
          QVariant variant;
          properties << "title" << "author" << "subtitle" << "mail" 
                     << "version" << "picture" << "footer" << "licence"
                     << "pictureCopyright" << "shadeColor" << "fontSize";
          foreach (property, properties)
            {
              sv = object.property(property.toLower());
              if (sv.isValid())
                {
                  variant = sv.toVariant();
                  setProperty(property.toStdString().c_str(), variant);
                }
            }
          
          // booktype property (always an array)
          sv = object.property("bookType");
          if (sv.isValid() && sv.isArray())
            {
              QStringList items;
              qScriptValueToSequence(sv, items); 
              setBookType(items);
            }

          // songs property (if not an array, the value can be "all")
          sv = object.property("songs");
          if (sv.isValid())
            {
              QStringList items;
              if (!sv.isArray())
                {
                  qDebug() << "not implemented yet";
                }
              else
                {
                  qScriptValueToSequence(sv, items); 
                }
              setSongs(items);
            }
        }
    }
  else
    {
      qWarning() << "unable to open file in read mode";
    } 
}

QWidget * CSongbook::panel()
{
  if (!m_panel)
    {
      m_panel = new QWidget;

      m_titleEdit = new QLineEdit(title());
      m_subtitleEdit = new QLineEdit(subtitle());
      m_authorEdit = new QLineEdit(author());
      m_versionEdit = new QLineEdit(version());
      m_mailEdit = new QLineEdit(mail());
      m_pictureEdit =new QLineEdit(QString("%1").arg(picture()));
      m_pictureCopyrightEdit = new QLineEdit(pictureCopyright());
      m_pictureEdit->setReadOnly(true);
    
      QToolButton *browsePictureButton = new QToolButton;
      browsePictureButton->setIcon(QIcon(":/icons/document-load.png"));
      connect(browsePictureButton, SIGNAL(clicked()),
	      this, SLOT(browsePicture()));
  
      m_shadeColorLabel = new QLabel;
      QColor shade(QString("#%1").arg(shadeColor()));
      m_shadeColorLabel->setText(shade.name());
      m_shadeColorLabel->setPalette(QPalette(shade));
      m_shadeColorLabel->setAutoFillBackground(true);
      
      QPushButton *pickShadeColorButton = new QPushButton(tr("Change"));
      connect(pickShadeColorButton, SIGNAL(clicked()),
	      this, SLOT(pickShadeColor()));  
      
      m_fontSizeSlider = new QSlider(Qt::Horizontal);
      m_fontSizeSlider->setRange(0,4);
      m_fontSizeSlider->setPageStep(1);
      m_fontSizeSlider->setSingleStep(1);
      m_fontSizeSlider->setTickPosition(QSlider::TicksBelow);
      m_fontSizeSlider->setValue(2);
      
      QGridLayout *layout = new QGridLayout;
      // title page
      layout->addWidget(new QLabel(tr("Title:")),0,0,1,1);
      layout->addWidget(m_titleEdit,0,1,1,3);
      layout->addWidget(new QLabel(tr("Subtitle:")),1,0,1,1);
      layout->addWidget(m_subtitleEdit,1,1,1,3);
      layout->addWidget(new QLabel(tr("Author:")),2,0,1,1);
      layout->addWidget(m_authorEdit,2,1,1,3);
      layout->addWidget(new QLabel(tr("Version:")),3,0,1,1);
      layout->addWidget(m_versionEdit,3,1,1,3);
      layout->addWidget(new QLabel(tr("Mail:")),4,0,1,1);
      layout->addWidget(m_mailEdit,4,1,1,3);
      layout->addWidget(new QLabel(tr("Picture:")),5,0,1,1);
      layout->addWidget(m_pictureEdit,5,1,1,2);
      layout->addWidget(browsePictureButton,5,3,1,1);
      layout->addWidget(new QLabel(tr("Copyright:")),6,0,1,1);
      layout->addWidget(m_pictureCopyrightEdit,6,1,1,3);
      // custom options
      layout->addWidget(new QLabel(tr("Shade Color:")),7,0,1,1);
      layout->addWidget(m_shadeColorLabel,7,1,1,2);
      layout->addWidget(pickShadeColorButton,7,3,1,1);
      layout->addWidget(new QLabel(tr("Shade Color:")),8,0,1,1);
      layout->addWidget(new QLabel(tr("small")),8,1,1,1);
      layout->addWidget(m_fontSizeSlider,8,2,1,1);
      layout->addWidget(new QLabel(tr("large")),8,3,1,1);
      m_panel->setLayout(layout);
    }
  return m_panel;
}

void CSongbook::pickShadeColor()
{
  QColor color = QColorDialog::getColor(QColor(), m_panel);
  m_shadeColorLabel->setText(color.name());
  m_shadeColorLabel->setPalette(QPalette(color));
}

void CSongbook::browsePicture()
{
  //todo: right now, only .jpg is supported since it's hardcoded in dockWidgets
  //problem is that in mybook.tex, there's just the basename so its extension 
  //should be guessed from somewhere else.
  QString filename = QFileDialog::getOpenFileName(m_panel, tr("Open Image File"),
						  QDir::home().path(),
						  tr("Images (*.jpg)"));
  if (!filename.isEmpty())
    m_pictureEdit->setText(filename);
}
