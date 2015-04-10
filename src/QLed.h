#ifndef QLED_H
#define QLED_H

#include <QtWidgets>

class QLed: public QWidget 
{
  Q_OBJECT
 public:
  
  QLed(QWidget *parent = 0);
  void setOnColor1(QColor _color) {m_OnColor1 = _color;}
  const QColor & getOnColor1() const {return m_OnColor1;}
  void setOnColor2(QColor _color) {m_OnColor2 = _color;}
  const QColor & getOnColor2() const {return m_OnColor2;}
  void setOffColor1(QColor _color) {m_OffColor1 = _color;}
  const QColor & getOffColor1() const {return m_OffColor1;}
  void setOffColor2(QColor _color) {m_OffColor2 = _color;}
  const QColor & getOffColor2() const {return m_OffColor2;}
  void setSize(int _width, int _height);
  
 public slots:
    void switchLed();

 protected:
    void paintEvent(QPaintEvent *);
 
 private:
    bool m_lit;
    QColor m_OnColor1;
    QColor m_OnColor2;
    QColor m_OffColor1;
    QColor m_OffColor2;
    int m_width;
    int m_height;
};
#endif
