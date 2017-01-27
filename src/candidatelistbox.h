#ifndef CANDIDATELISTBOX_H
#define CANDIDATELISTBOX_H

#include <qframe.h>
#include <qstringlist.h>
#include <qpoint.h>

class QListBox;
class QListBoxItem;
class QLabel;

class CandidateListBox : public QFrame {
  Q_OBJECT

public:
  enum {
    MAX_VISIBLE_ITEMS = 9,
  };

  CandidateListBox();
  void showList(const QStringList& strs, int crntidx = 0);
  void selectCandidate(int idx);
  void selectPreviousCandidate(uint decrement = 1);
  void selectNextCandidate(uint increment = 1);
  void setFont(const QFont&);
  QString currentText() const;
  QString text(int index) const;
  int  topItem() const;
  uint count() const;

  bool isListVisible() const;
  bool isPredictionsVisible() const;
  QString candidatePredicted() const;

public slots:
  void setPos(const QPoint& p);
  void showCandidatePredicted(const QString& str);

signals:
  void clicked(QListBoxItem* item);

protected:
  void showEvent(QShowEvent *);
  void hideEvent(QHideEvent *);

protected slots:
  void setLabelValue(int d);
  void adjustDisplay();

private:
  QListBox*    _lbl;
  QListBox*    _lstbox;
  QLabel*      _crntitem;
  QLabel*      _predictxt;
  QPoint       _pos;
};

#endif // CANDIDATELISTBOX_H
