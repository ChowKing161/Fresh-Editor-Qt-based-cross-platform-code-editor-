#include <QtWidgets>
#include <QDebug>
#include "codeeditor.h"
#include "qnamespace.h"

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this,SIGNAL(cursorPositionChanged()),this,SLOT(showCompleteWidget()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    //设置颜色值
    lineColor.setRgb(56,60,69);
    //editorColor.setRgb(34,39,49);
    editorColor.setRgb(0,0,0);

    //设置背景色
    QPalette p = this->palette();
    p.setColor(QPalette::Active, QPalette::Base, editorColor);
    p.setColor(QPalette::Inactive, QPalette::Base, editorColor);
    p.setColor(QPalette::Text,Qt::white);
    this->setPalette(p);
    //初始化补全列表
    setUpCompleteList();
    completeWidget= new Completer(this);
    completeWidget->hide();
    completeWidget->setMaximumHeight(fontMetrics().height()*5);
    completeState=CompleteState::Hide;
}

//注意:因为行号的长度是变化的,所以显示行号的Qwidget的宽度也应该是变化的,因此要获取QPlainTextEdit的所有行数目.
int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;//行数数字的位数
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    if(digits<3)digits=3;
    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

//设定左边留白的宽度，参数无效，没有用到
void CodeEditor::updateLineNumberAreaWidth(int  /*newBlockCount*/ )
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//文本框滚动时同时滚动行数
void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    /*if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);*/

}

//尺寸调整函数
void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor CurrentLineColor = QColor(Qt::red).lighter(160);

        selection.format.setBackground(CurrentLineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

//打印行号，此函数被paintEvent 调用，paintEvent在头文件里被重写

//行号的显示其实是在QPlainTextEdit左面放置一个QWidget,然后在Qwidget上画出对应的行号.
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    //1.设置绘图区域
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), lineColor);
    //2.设置上下边界
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    //3.设置画笔颜色，并每次一个字符高度为单位循环向下绘制行数
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::lightGray);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

//自动补全
void CodeEditor::keyPressEvent(QKeyEvent *event){
    //匹配括号
  if(event->modifiers()==Qt::ShiftModifier&&event->key()==Qt::Key_ParenLeft){
      this->insertPlainText(tr("()"));
      this->moveCursor(QTextCursor::PreviousCharacter);
    }
    //匹配注释
  /*else if(event->modifiers()==Qt::ShiftModifier&&event->key()==Qt::Key_QuoteDbl){
      this->insertPlainText(tr("\"\""));
      this->moveCursor(QTextCursor::PreviousCharacter);
    }*/
  else if(event->key()==Qt::Key_Up&&completeState==CompleteState::Showing){
      if(completeWidget->currentRow()>0)
        completeWidget->setCurrentRow(completeWidget->currentRow()-1);
    }
  else if(event->key()==Qt::Key_Down&&(completeState==CompleteState::Showing)){
      if(completeWidget->currentRow()<completeWidget->count()-1)
        completeWidget->setCurrentRow(completeWidget->currentRow()+1);
    }
  else if(event->key()==Qt::Key_Return&&(completeState==CompleteState::Showing)){
      QString insertText=completeWidget->currentItem()->text();
      QString word=this->getWordOfCursor();
      completeState=CompleteState::Ignore;
      for(int i=0;i<word.count();++i)
        this->textCursor().deletePreviousChar();
      this->insertPlainText(insertText);
      if(insertText.contains(tr("#include")))
        this->moveCursor(QTextCursor::PreviousCharacter);  //包含头文件，将光标移至“”或<>中间
      completeState=CompleteState::Hide;
      completeWidget->hide();
    }//*
  else if(event->key()==Qt::Key_Return){//回车下行层级自动缩进功能
      //获得本行的文本
      QString temp=this->document()->findBlockByLineNumber(this->textCursor().blockNumber()).text();
      QPlainTextEdit::keyPressEvent(event);
      if(temp.count()<=0)return;
      //输出回车那一行的前距
      foreach(const QChar &c,temp){
          if(c.isSpace())this->insertPlainText(c);
          else break;
        }
      //如果是for() while() switch() if()则缩进一个tab,一种粗略地做法可能会出错
      if(temp.at(temp.count()-1)==')'&&(temp.contains(tr("for("))||temp.contains(tr("while("))
                                        ||temp.contains(tr("switch("))||temp.contains(tr("if("))))
          this->insertPlainText(tr("\t"));
      //如果是{ 则缩进并补}
      if(temp.at(temp.count()-1)=='{'){
          this->insertPlainText(tr("\t"));
          QTextCursor cursor=this->textCursor();
          int pos=this->textCursor().position();
          this->insertPlainText(tr("\n"));
          foreach(const QChar &c,temp){
              if(c.isSpace())this->insertPlainText(c);
              else break;
            }
          this->insertPlainText(tr("}"));
          cursor.setPosition(pos);
          this->setTextCursor(cursor);//返回中间一行
        }
    }//*/
  else if(event->key()==Qt::Key_Backspace){
      switch(this->document()->characterAt(this->textCursor().position()-1).toLatin1()){
        case '(':
          QPlainTextEdit::keyPressEvent(event);
          if(this->document()->characterAt(this->textCursor().position())==')'){
              this->textCursor().deleteChar();
            }break;
        case '\"':
          QPlainTextEdit::keyPressEvent(event);
          if(this->document()->characterAt(this->textCursor().position())=='\"'){
              this->textCursor().deleteChar();
            }break;
        case '<':
          QPlainTextEdit::keyPressEvent(event);
          if(this->document()->characterAt(this->textCursor().position())=='>'){
              this->textCursor().deleteChar();
            }break;
        default:
          QPlainTextEdit::keyPressEvent(event);
        }
    }
  else{
    QPlainTextEdit::keyPressEvent(event);
    }
}
void CodeEditor::setUpCompleteList(){
  completeList<< "char" << "class" << "const" << "cin" << "cout"
              << "double" << "enum" << "explicit"
              << "friend" << "inline" << "int"
              << "long" << "namespace" << "operator"
              << "private" << "protected" << "public"
              << "short" << "signals" << "signed"
              << "slots" << "static" << "struct"
              << "template" << "typedef" << "typename"
              << "union" << "unsigned" << "virtual"
              << "void" << "volatile" << "bool"<<"using"<<"constexpr"
              <<"sizeof"<<"if"<<"for"<<"foreach"<<"while"<<"do"<<"case"
              <<"break"<<"continue"<<"template"<<"delete"<<"new"
              <<"default"<<"try"<<"return"<<"throw"<<"catch"<<"goto"<<"else"
              <<"extren"<<"this"<<"switch"<<"#include <>"<<"#include \"\""<<"#define"<<"iostream"
              <<"std";
}
//获取当前光标位置的字符串
QString CodeEditor::getWordOfCursor(){
  int pos=this->textCursor().position()-1;
  QVector<QChar> words;
  QString result;
  QChar ch=this->document()->characterAt(pos+1);
  if(ch.isDigit()||ch.isLetter()||ch==' ')return result;
  ch=this->document()->characterAt(pos);
  if(ch==' ')return result;
  while(ch.isDigit()||ch.isLetter()||ch=='_'||ch=='#'){
      words.append(ch);
      pos--;
      ch=this->document()->characterAt(pos);
    }
  for(int i=words.size()-1;i>=0;i--)
    result+=words[i];
  return result;

}

void CodeEditor::showCompleteWidget(){
  if(completeState==CompleteState::Ignore)return;//忽略光标和文本变化的响应,避免陷入事件死循环和互相钳制
  completeWidget->hide();
  completeState=CompleteState::Hide;
  QString word=this->getWordOfCursor();
  completeWidget->clear();
  if(!word.isEmpty()){//光标所在单词是不是合法(能不能联想)
      int maxSize=0;
      QMap<QString,int> distance;
      vector<QString> itemList;
      foreach(const QString &temp,completeList){
          if(temp.contains(word)){
              itemList.push_back(temp);
              distance[temp]=Completer::ldistance(temp.toStdString(),word.toStdString());
              if(temp.length()>maxSize)maxSize=temp.length();
            }
        }
      //有没有匹配的字符
      if(itemList.size()>0){//如果有的话
          //按单词长短，自上而下升序排列
      sort(itemList.begin(),itemList.end(),[&](const QString &s1,const QString &s2)->bool{return distance[s1]<distance[s2]; });
      foreach(const QString& item,itemList){
          completeWidget->addItem(new QListWidgetItem(item));
        }

      int x=this->getCompleteWidgetX();
      int y=this->cursorRect().y()+fontMetrics().height();

      completeWidget->move(x,y);
      if(completeWidget->count()>5)completeWidget->setFixedHeight(fontMetrics().height()*6);
      else completeWidget->setFixedHeight(fontMetrics().height()*(completeWidget->count()+1));
      completeWidget->setFixedWidth((fontMetrics().width(QLatin1Char('9'))+6)*maxSize);
      completeWidget->show();
      completeState=CompleteState::Showing;
      completeWidget->setCurrentRow(0,QItemSelectionModel::Select);
        }
    }

}
//completewidget的横坐标在 被匹配单词的起始处
//求出listwidget的横坐标
int CodeEditor::getCompleteWidgetX(){
  QTextCursor cursor=this->textCursor();
  int pos=cursor.position()-1;
  int origianlPos=pos+1;
  QChar ch;
  ch=this->document()->characterAt(pos);
  while((ch.isDigit()||ch.isLetter()||ch=='_'||ch=='#')&&pos>0){
      pos--;
      ch=this->document()->characterAt(pos);
    }
  pos++;
  completeState=CompleteState::Ignore;
  cursor.setPosition(pos);
  this->setTextCursor(cursor);
  int x=this->cursorRect().x()+2*fontMetrics().width(QLatin1Char('9'));
  cursor.setPosition(origianlPos);
  this->setTextCursor(cursor);
  completeState=CompleteState::Hide;
  return x;
}
