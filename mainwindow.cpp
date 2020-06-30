#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include<QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  firstLoad=true;
  ui->setupUi(this);
  setUpHighlighter();
  //init status bar
  ui->outputText->parentWindow=this;
  ui->statusBar->showMessage(tr("Ready"));
  //初始化工具栏
  ui->mainToolBar->setMovable(false);
  ui->mainToolBar->setStyleSheet("QToolButton:hover {background-color:darkgray} QToolBar {background: rgb(82,82,82);border: none;}");
  //初始化图标设置
  runIcon.addPixmap(QPixmap(":/image/Run.png"));
  stopIcon.addPixmap(QPixmap(":/image/stop.png"));
  //初始化窗口颜色
  QPalette windowPalette=this->palette();
  windowPalette.setColor(QPalette::Active,QPalette::Window,QColor(82,82,82));
  windowPalette.setColor(QPalette::Inactive,QPalette::Window,QColor(82,82,82));
  this->setPalette(windowPalette);
  //事件处理
  initFileData();
  connect(ui->actionNewFile,SIGNAL(triggered(bool)),this,SLOT(newFile()));
  connect(ui->actionOpen,SIGNAL(triggered(bool)),this,SLOT(openFile()));
  connect(ui->actionSave_File,SIGNAL(triggered(bool)),this,SLOT(saveFile()));
  connect(ui->action_Unicode,SIGNAL(triggered(bool)),this,SLOT(saveUNICODE()));
  connect(ui->action_ANSI,SIGNAL(triggered(bool)),this,SLOT(saveANSI()));
  connect(ui->actionUndo,SIGNAL(triggered(bool)),this,SLOT(undo()));
  connect(ui->actionRedo,SIGNAL(triggered(bool)),this,SLOT(redo()));
  connect(ui->editor,SIGNAL(textChanged()),this,SLOT(changeSaveState()));
  connect(ui->actionRun,SIGNAL(triggered(bool)),this,SLOT(run()));
  connect(ui->actionFind,SIGNAL(triggered(bool)),this,SLOT(find()));
  connect(&process,SIGNAL(finished(int)),this,SLOT(runFinished(int)));
  connect(&process,SIGNAL(readyReadStandardOutput()),this,SLOT(updateOutput()));
  connect(&process,SIGNAL(readyReadStandardError()),this,SLOT(updateError()));
  connect(ui->actionAbout,SIGNAL(triggered(bool)),this,SLOT(about()));
  fileSaved=true;
}

MainWindow::~MainWindow()
{
  delete ui;
}
void MainWindow::setUpHighlighter(){
  QFont font;
  font.setFamily("Courier");
  font.setFixedPitch(true);
  //font.setPointSize(20);
  ui->editor->setFont(font);
  ui->editor->setTabStopWidth(fontMetrics().width(QLatin1Char('9'))*4);
  highlighter=new Highlighter(ui->editor->document());
}

void MainWindow::resizeEvent(QResizeEvent *event){
  QMainWindow::resizeEvent(event);
  ui->editor->setGeometry(10,0,width()-20,height()-ui->statusBar->height()-ui->mainToolBar->height()-80-15);
  ui->outputText->setGeometry(10,ui->editor->height()+10,this->width()-20,80);
}
void MainWindow::initFileData(){
  fileName=tr("Untitled.cpp");
  filePath=tr("~/Desktop/Untitled.cpp");
  fileSaved=true;
  isRunning=false;
}
void MainWindow::undo(){
  ui->editor->undo();
}
void MainWindow::redo(){
  ui->editor->redo();
}
void MainWindow::find(){
    QDialog *findDialog = new QDialog(this);
    findDialog->setWindowTitle(QStringLiteral("查找"));
    find_textLineEdit = new QLineEdit(findDialog);
    find_replaceLineEdit = new QLineEdit(findDialog);
    QPushButton *find_BtnForward = new QPushButton(QStringLiteral("查找下一个"), findDialog);
    QPushButton *find_BtnBackforward = new QPushButton(QStringLiteral("查找上一个"), findDialog);
    QPushButton *replace_Btn = new QPushButton(QStringLiteral("替换"), findDialog);
    QVBoxLayout *layout = new QVBoxLayout(findDialog);
    QLabel *str1 = new QLabel(QStringLiteral("请输入查询的内容："));
    QLabel *str2 = new QLabel(QStringLiteral("请输入替换的内容："));
    layout->addWidget(str1);
    layout->addWidget(find_textLineEdit);
    layout->addWidget(find_BtnForward);
    layout->addWidget(find_BtnBackforward);
    layout->addWidget(str2);
    layout->addWidget(find_replaceLineEdit);
    layout->addWidget(replace_Btn);
    findDialog->show();
    connect(find_BtnForward, SIGNAL(clicked()), this, SLOT(show_findTextForward()));
    connect(find_BtnBackforward, SIGNAL(clicked()), this, SLOT(show_findTextBackward()));
    connect(replace_Btn, SIGNAL(clicked()), this, SLOT(replace_findText()));
}
void MainWindow::show_findTextForward() //向后查找
{
    QString findtext=find_textLineEdit->text();//获得对话框的内容
    if(ui->editor->find(findtext))//查找后一个
    {
        // 查找到后高亮显示
        QPalette palette = ui->editor->palette();
        palette.setColor(QPalette::Highlight,QColor(255,255,110));
        palette.setColor(QPalette::HighlightedText,QColor(0,0,200));
        ui->editor->setPalette(palette);
    }
    else
    {
        QMessageBox::information(this,QStringLiteral("注意"),QStringLiteral("没有找到内容"),QMessageBox::Ok);
    }
}
void MainWindow::show_findTextBackward()//向前查找
{
    QString findtext=find_textLineEdit->text();//获得对话框的内容
    if(ui->editor->find(findtext, QTextDocument::FindBackward))//查找上一个
    {
        // 查找到后高亮显示
        QPalette palette = ui->editor->palette();
        palette.setColor(QPalette::Highlight,QColor(255,255,110));
        palette.setColor(QPalette::HighlightedText,QColor(0,0,200));
        ui->editor->setPalette(palette);
    }
    else
    {
        QMessageBox::information(this,QStringLiteral("注意"),QStringLiteral("没有找到内容"),QMessageBox::Ok);
    }
}
void MainWindow::replace_findText() //替换查找内容
{
    QString replacetext = find_replaceLineEdit->text();//获得对话框的内容
    QString SelectedText = ui->editor->textCursor().selectedText();
    if(ui->editor->textCursor().hasSelection())
    {
        ui->editor->textCursor().insertText(replacetext);
    }
    else
    {
        QMessageBox::information(this,QStringLiteral("注意"),QStringLiteral("请先选中要替换的内容"),QMessageBox::Ok);
    }
}

void MainWindow::saveFile(){
  QString savePath=QFileDialog::getSaveFileName(this,tr("选择保存路径与文件名"),fileName,tr("Cpp File(*.cpp *.c *.h)"));
  if(!savePath.isEmpty()){
      QFile out(savePath);
      out.open(QIODevice::WriteOnly|QIODevice::Text);
      QTextStream str(&out);
      str<<ui->editor->toPlainText();
      out.close();
      fileSaved=true;
      QRegularExpression re(tr("(?<=\\/)\\w+\\.cpp|(?<=\\/)\\w+\\.c|(?<=\\/)\\w+\\.h"));
      fileName=re.match(savePath).captured();
      filePath=savePath;
      this->setWindowTitle(tr("Fresh Editor - ")+fileName);
    }
}
void MainWindow::saveUNICODE()  //保存为unicode编码格式
{
  QString savePath=QFileDialog::getSaveFileName(this,QStringLiteral("选择保存路径与文件名"),
                                                fileName,tr("Cpp File(*.cpp *.c *.h)"));
  if(!savePath.isEmpty())
  {
      QFile out(savePath);
      out.open(QIODevice::WriteOnly|QIODevice::Text);

      QTextStream stream(&out);
      stream.setCodec("unicode");
      stream<<ui->editor->toPlainText();
      out.close();

      fileSaved=true;

      QRegularExpression RE(tr("(?<=\\/)\\w+\\.cpp|(?<=\\/)\\w+\\.c|(?<=\\/)\\w+\\.h"));
      fileName=RE.match(savePath).captured();
      filePath=savePath;
      this->setWindowTitle(tr("Fresh Editor - ")+fileName);
    }
}

void MainWindow::saveANSI() //保存为ansi格式
{
  QString savePath=QFileDialog::getSaveFileName(this,QStringLiteral("选择保存路径与文件名"),
                                                fileName,tr("Cpp File(*.cpp *.c *.h)"));
  if(!savePath.isEmpty())
  {
      QFile output(savePath);
      output.open(QIODevice::WriteOnly|QIODevice::Text);

      QTextStream stream(&output);
      stream.setCodec("ansi");
      stream<<ui->editor->toPlainText().toLocal8Bit();
      output.close();

      fileSaved=true;

      QRegularExpression re(tr("(?<=\\/)\\w+\\.cpp|(?<=\\/)\\w+\\.c|(?<=\\/)\\w+\\.h"));
      fileName=re.match(savePath).captured();
      filePath=savePath;
      this->setWindowTitle(tr("Fresh Editor - ")+fileName);
    }
}
void MainWindow::newFile(){
  MainWindow *newWindow=new MainWindow();
  QRect newPos=this->geometry();
  newWindow->setGeometry(newPos.x()+10,newPos.y()+10,newPos.width(),newPos.height());
  newWindow->show();
}
void MainWindow::openFile(){
  if(!fileSaved){
      if(QMessageBox::Save==QMessageBox::question(this,tr("文件未保存"),tr("当前文件没有保存，是否保存？"),QMessageBox::Save,QMessageBox::Cancel))
        saveFile();
    }
  QString openPath=QFileDialog::getOpenFileName(this,tr("选择要打开的文件"),filePath,tr("Cpp File(*.cpp *.c *.h)"));
  if(!openPath.isEmpty()){
      QFile in(openPath);
      in.open(QIODevice::ReadOnly|QIODevice::Text);
      QTextStream str(&in);
      ui->editor->setPlainText(str.readAll());
      QRegularExpression re(tr("(?<=\\/)\\w+\\.cpp|(?<=\\/)\\w+\\.c|(?<=\\/)\\w+\\.h"));
      fileName=re.match(openPath).captured();
      this->setWindowTitle(tr("Fresh Editor - ")+fileName);
      filePath=openPath;
      fileSaved=true;
    }
}
void MainWindow::run(){
  if(isRunning){
      process.terminate();
      ui->actionRun->setIcon(runIcon);
      return;
    }
  if(!fileSaved){
      if(QMessageBox::Save==QMessageBox::question(this,tr("文件未保存"),tr("文件保存后才能运行，是否保存？"),QMessageBox::Save,QMessageBox::Cancel))
        saveFile();
    }
  if(fileSaved){
    //if(process!=nullptr)delete process;
    isRunning=true;
    ui->statusBar->showMessage(tr("程序运行中..."));
    ui->outputText->clear();
    output.clear();
    error.clear();
    QString buildPath;
    QRegularExpression re(tr(".*(?=\\.cpp)|.*(?=\\.c)|.*(?=\\.h)"));
    buildPath=re.match(filePath).captured();
    //ui->label->setText(buildPath);
    //ui->label->setText('\n'+fileName);
    //ui->label->setText('\n'+buildPath);
    //qDebug()<<buildPath;
    //process.start(QString(tr("g++ ")+filePath+tr(" -o ")+buildPath));
    //process.start("bash", QStringList() << QString(tr("g++ ")+filePath+tr(" -o ")+buildPath+tr(";")+buildPath));
    //process.start(QString(tr("g++ ")+filePath+tr(" -o ")+buildPath));
    //process.start(QString(tr("g++ ")+filePath+tr(" -o ")+buildPath+tr("\n")+tr("F:/1.exe")));
    process.start("cmd", QStringList() << "/c" << QString(tr("g++ ")+filePath+tr(" -o ")+buildPath+tr("&&")+buildPath));
    process.waitForStarted();
    ui->outputText->setFocus();
    ui->actionRun->setIcon(stopIcon);
    }
}
void MainWindow::runFinished(int code){
  ui->actionRun->setIcon(runIcon);
  isRunning=false;
  qDebug()<<tr("exit code=")<<code;
  ui->statusBar->showMessage(tr("Ready"));
}
void MainWindow::updateOutput(){
  output=QString::fromLocal8Bit(process.readAllStandardOutput());
  //ui->outputText->setPlainText(output+tr("\n")+error);
  ui->outputText->setPlainText(ui->outputText->toPlainText()+output);//+tr("\n"));
}
void MainWindow::updateError(){
  error=QString::fromLocal8Bit(process.readAllStandardError());
  //ui->outputText->setPlainText(output+tr("\n")+error);
  ui->outputText->setPlainText(ui->outputText->toPlainText()+error);//+tr("\n"));
  process.terminate();
  isRunning=false;
}
void MainWindow::inputData(QString data){
  if(isRunning)process.write(data.toLocal8Bit());
}
void MainWindow::closeEvent(QCloseEvent *event){
  if(!fileSaved){
      if(QMessageBox::Save==QMessageBox::question(this,tr("未保存就要退出？"),tr("当前文件没有保存，是否保存？不保存文件改动将会丢失"),QMessageBox::Save,QMessageBox::Cancel))
        saveFile();
      fileSaved=true;
    }
}
void MainWindow::about(){
  QMessageBox::information(this,tr("关于"),tr(" Fresh-Editor v1.0 \n 感谢使用 \n 反馈信息联系：chowking161@163.com"),QMessageBox::Ok);
}
