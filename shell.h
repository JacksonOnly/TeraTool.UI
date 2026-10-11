#pragma once
#include <QtWidgets>

class MainForm : public QMainWindow {
 Q_OBJECT
 public: explicit MainForm(QWidget *parent=nullptr):QMainWindow(parent) {setWindowFlags(Qt::Widget);} };
class CentralWidget : public QMainWindow {
 Q_OBJECT
 public: using QMainWindow::QMainWindow; };
class QGoodCentralWidget : public QWidget {
 Q_OBJECT
 public: using QWidget::QWidget; };
class TitleBar : public QFrame {
 Q_OBJECT
 public: using QFrame::QFrame;
 protected: void mousePressEvent(QMouseEvent *e) override { origin=e->globalPos()-window()->pos(); }
 void mouseMoveEvent(QMouseEvent *e) override {if(e->buttons()&Qt::LeftButton&&!window()->isMaximized())window()->move(e->globalPos()-origin);}
 void mouseDoubleClickEvent(QMouseEvent*) override {window()->isMaximized()?window()->showNormal():window()->showMaximized();}
 private: QPoint origin;
};
class TitleWidget : public QWidget {
 Q_OBJECT
 public: explicit TitleWidget(QWidget *parent=nullptr):QWidget(parent) {setAttribute(Qt::WA_TransparentForMouseEvents);}
 protected: void paintEvent(QPaintEvent*) override {QPainter p(this);p.setPen(palette().color(QPalette::WindowText));p.setFont(font());p.drawText(rect(),Qt::AlignCenter,window()->windowTitle());}
};
class IconWidget : public QWidget {
 Q_OBJECT
 public: using QWidget::QWidget;
 protected: void paintEvent(QPaintEvent*) override {QPainter p(this);window()->windowIcon().paint(&p,QRect((width()-16)/2,(height()-16)/2,16,16));}
};
class CaptionButton : public QWidget {
 Q_OBJECT
 Q_PROPERTY(QString kind READ captionKind WRITE setKind)
 public: using QWidget::QWidget; QString kind;
 QString captionKind() const {return kind;}
 void setKind(QString value) {kind=value;update();}
 protected: void paintEvent(QPaintEvent*) override {
  QPainter p(this);const bool dark=window()->property("darkTheme").toBool();
  if(underMouse())p.fillRect(rect(),kind=="close"?QColor("#e81123"):QColor(dark?"#424242":"#dce3ec"));
  p.setPen(QPen(underMouse()&&kind=="close"?QColor("#ffffff"):palette().color(QPalette::WindowText),1));
  const int x=(width()-10)/2,y=(height()-10)/2;
  if(kind=="close"){p.drawLine(x,y,x+10,y+10);p.drawLine(x+10,y,x,y+10);}
  else if(kind=="minimize")p.drawLine(x,y+5,x+10,y+5);
  else if(window()->isMaximized()){p.drawRect(x+2,y,8,8);p.fillRect(QRect(x,y+2,8,8),palette().color(QPalette::Window));p.drawRect(x,y+2,8,8);}
  else p.drawRect(x,y,10,10);
 }
 void enterEvent(QEvent*) override {update();} void leaveEvent(QEvent*) override {update();}
 void mouseReleaseEvent(QMouseEvent *e) override {if(e->button()!=Qt::LeftButton||!rect().contains(e->pos()))return;if(kind=="close")window()->close();else if(kind=="minimize")window()->showMinimized();else window()->isMaximized()?window()->showNormal():window()->showMaximized();update();}
};
