#ifndef PREEDITAREA_H
#define PREEDITAREA_H

#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qfont.h>

class QLabel;

class PreeditArea : public QWidget {
  Q_OBJECT

public:
  PreeditArea();
  virtual ~PreeditArea() { }

  void setFont(const QFont &);
  void showInputingString(const QString& str, uint caret_pos);
  void showConvertingSegments(const QStringList& strlist, uint attention=0);
  void showChangingSegmentLength(const QStringList& strlist, uint attention=0);
  void movePreeditPos(const QPoint& pos);
  QString text() const;
  QString backwardText() const;
  QString attentionText() const;
  QString forwardText() const;

public slots:
  void polish();
  void init();
  void adjustSize();
  void hide();

signals:
  void listPointChanged(const QPoint& pos);

protected:
  void showText(const QStringList& strlist, uint attention, uint caret_pos=0);
  void readColorSetting();

private:
  enum State {
    Input = 0,
    Attention,
    Converted,
    Changing,
    NumStates,
  };

  enum {
    FgInput = 0,      // ����ʸ����
    BgInput,          // �����طʿ� 
    FgAttention,      // ����ʸ��ʸ����
    BgAttention,      // ����ʸ���طʿ�
    FgConverted,      // �Ѵ��Ѥ�ʸ��ʸ����
    BgConverted,      // �Ѵ��Ѥ�ʸ���طʿ�
    FgChanging,       // ʸ��Ĺ�ѹ�ʸ����
    BgChanging,       // ʸ��Ĺ�ѹ��طʿ�
    NumColors,
  };

  QLabel*  _attentseg;        // �Ѵ��Ѥ�����ʸ��
  QLabel*  _backseg;          // �Ѵ��Ѥ�������ʸ�Ẹ
  QLabel*  _forwardseg;       // �Ѵ��Ѥ�������ʸ�ᱦ
  QLabel*  _caret;            // �����å�
  QColor   _colorlist[NumColors];
  QFont    _font[NumStates];
  State    _state;
};

#endif // PREEDITAREA_H
