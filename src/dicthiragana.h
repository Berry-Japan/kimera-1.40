#ifndef DICTHIRAGANA_H
#define DICTHIRAGANA_H

#include "kimeraglobal.h"
#include "inputmode.h"
#include <qdict.h>
#include <qstring.h>
#include <qevent.h>
#include <qptrlist.h>

class DictHiragana {
public:
  static const QKeyEvent  SEPARATOR_CHAR;
  
  DictHiragana();
  ~DictHiragana();

  // �����
  void     init();

  // ���� �� ��ߤ��� �Ѵ�
  QString  convert(const QPtrList<QKeyEvent>& kevlst, const InputMode& mode, bool tenkey_hankaku=FALSE) const;

  // ��ߤ��� �� �������� ���Ѵ�  
  void     reverse(const QString& yomi, const InputMode& mode, bool tenkey_hankaku, QPtrList<QKeyEvent>& kevlst) const;

  // �Ҥ餬�� �� ��ߤ���(���޻�) �Ѵ�
  QString  convertString(const QString& hira, const InputMode& mode=InputMode()) const;
  
protected:
  void     initDict(QDict<QString>& dict, const QString&, bool reverse_dict=FALSE);
  QString  convertKey(const QKeyEvent& key, const InputMode& mode) const;  // ���� �� ��ߤ��� �Ѵ�
  QString  convertToHira(const QString&) const;    // �Ҥ餬���Ѵ�
  QString  convertToKata(const QString&) const;    // ���������Ѵ�
  QString  convertToHankakuKana(const QString& src) const;   // Ⱦ�ѥ����Ѵ�
  QString  convertToZenkakuEisu(const QString&) const;  // ���ѱѿ��Ѵ�
  QString  reverseConvt(const QString& hira) const;     // �Ҥ餬��(��������) �� ���޻� ���Ѵ�  
  QString  replaceDakutenChar(const QString& str) const;  // ����Ⱦ����ʸ���ִ�

  static void copyDictFile(const QString dicname);

private:
  QDict<QString>     _dicthira;      // �Ҥ餬�ʼ���
  QDict<QString>     _dictkata;      // �������ʼ���
  QDict<QString>     _dicthankana;   // Ⱦ�ѥ��ʼ���
  QDict<QString>     _dictalphbt;    // ���ѱѻ�����
  QDict<QString>     _dictsymbol;    // ���ѿ������漭��
  QDict<QString>     _dictkanainp;   // �������ϼ���
  QDict<QString>     _dictdakuten;   // ����������������
  QDict<QString>     _reversedict;   // �հ�������
};

#endif // DICTHIRAGANA_H
