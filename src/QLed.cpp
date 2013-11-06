#include <QtGui>

#include "QLed.h"

QLed::QLed(QWidget *parent) : QWidget(parent), 
			      m_OnColor1(QColor(0,255,0)), m_OnColor2(QColor(0,192,0)),
			      m_OffColor1(QColor(0,28,0)), m_OffColor2(QColor(0,128,0)),
			      m_width(20), m_height(20)
{
  setMinimumHeight(m_height);
  setMinimumWidth(m_width);
  adjustSize();
  m_lit = false;
}

void QLed::setSize(int _width, int _height)
{
  m_height = _height;
  m_width  = _width;
  
  setMinimumHeight(m_height);
  setMinimumWidth(m_width);
  adjustSize();
}

void QLed::paintEvent(QPaintEvent *) 
{
  qreal realSize = qMin(width(), height());
  qreal scaledSize = 1000;
 
  QRadialGradient gradient;
  QPainter painter(this);
  QPen pen(Qt::black);
  
  pen.setWidth(1);
  
  painter.setRenderHint(QPainter::Antialiasing);
  painter.translate(width()/2, height()/2);
  painter.scale(realSize/scaledSize, realSize/scaledSize);
  
  gradient = QRadialGradient(QPointF(-500,-500), 1500, QPointF(-500,-500));
  gradient.setColorAt(0, QColor(224,224,224));
  gradient.setColorAt(1, QColor(28,28,28));
  painter.setPen(pen);
  painter.setBrush(QBrush(gradient));
  painter.drawEllipse(QPointF(0,0), 500, 500);
  
  gradient = QRadialGradient(QPointF(500,500), 1500, QPointF(500,500));
  gradient.setColorAt(0, QColor(224,224,224));
  gradient.setColorAt(1, QColor(28,28,28));
  painter.setPen(pen);
  painter.setBrush(QBrush(gradient));
  painter.drawEllipse(QPointF(0,0), 450, 450);
  
  painter.setPen(pen);
  if (m_lit) {
    gradient = QRadialGradient(QPointF(-500,-500), 1500, QPointF(-500,-500));
    gradient.setColorAt(0, m_OnColor1);
    gradient.setColorAt(1, m_OnColor2);
  } else {
    gradient = QRadialGradient(QPointF(500,500), 1500, QPointF(500,500));
    gradient.setColorAt(0, m_OffColor1);
    gradient.setColorAt(1, m_OffColor2);
  }
  painter.setBrush(gradient);
  painter.drawEllipse(QPointF(0,0), 400, 400);
}

void QLed::switchLed() 
{
  m_lit = !m_lit;
  repaint();
  qDebug() << "QLed: switchLed";
}
