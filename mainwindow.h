#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "highlighter.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QLineEdit>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
private:
  QIcon runIcon;
  QIcon stopIcon;
  Ui::MainWindow *ui;
  Highlighter *highlighter;
  QProcess process;
  void setUpHighlighter();
  //---------记录文件信息----------
  QString fileName;
  QString filePath;
  bool fileSaved;
  bool isRunning;
  //bool fileEdited;
  void initFileData();
  bool firstLoad;
  QLineEdit *find_textLineEdit;
  QLineEdit *find_replaceLineEdit;
  //-----------------------------


  //---------code running data---
  QString output;
  QString error;
  //-----------------------------
public slots:
  void changeSaveState(){
    //qDebug()<<"changed";
    if(firstLoad&&fileSaved){
        this->setWindowTitle(tr("Fresh-Editor ")+fileName);
        firstLoad=false;
        return;
      }
    fileSaved=false;
    this->setWindowTitle(tr("Fresh-Editor ")+fileName+tr("*"));
  }

  //---------工具栏响应函数---------
  void newFile();
  void saveFile();
  void saveUNICODE();
  void saveANSI();
  void openFile();
  void undo();
  void redo();
  void run();
  void find();
  void show_findTextForward();
  void show_findTextBackward();
  void replace_findText();
  //------------------------------
  void runFinished(int code);
  void updateOutput();
  void updateError();
  void about();
public:
  void inputData(QString data);
protected:
  void resizeEvent(QResizeEvent* event)override;
  void closeEvent(QCloseEvent* event)override;
};

#endif // MAINWINDOW_H
