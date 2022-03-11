/****************************************************************************
**
** Copyright (C) 2020 The Colibri Company Ltd.
**
****************************************************************************/

#ifndef OCRWINDOW_H
#define OCRWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTranslator>
#ifndef QT_NO_PRINTER
#include <QTreeView>
#include <QGraphicsView>
#include <QFileSystemModel>
#include <QSplitter>
#include <QHBoxLayout>
#include <QImageReader>
#endif
#include "customscene.h"
#include "customview.h"
#include <QMessageBox>
#include <QUndoGroup>
#include <QIntValidator>
#include <QLineEdit>
QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QDirModel;
class QComboBox;
//class Task;
QT_END_NAMESPACE

//! [0]
class OCRWindow : public QMainWindow
{
    Q_OBJECT

public:
    OCRWindow();
signals:
    /**
      *信号必须要signals关键字来声明
      * 信号没有返回值，但可以有参数
      * 信号就是函数声明，无需定义
      * 使用：emit mySignal();
      * 信号可以被重载
    */
    void goBack();
private slots:
    void goHome();
    void openFolder();
    void panImage(bool checked);
    void drawPloyItem(bool checked);
    void fitViewToWindow();
    void fitViewToActual();
    void fullScreen();
    void nextImage();
    void prevImage();
    void help();
    void about();
    void selectFile();
    void onFileSelected(const QItemSelection& selected, const QItemSelection& deselected);
    void updateLabelImageSize(QSize imageSize);
    void updateLabelCursorPos(QPointF cursorPos);
    void updateBoxInfo(QRect rect, QString typeName);
    void switchLanguage();
    void saveImageNamesToFile(const QString fileName);
    void updateCopyCutActions();
    void updatePasteAction();
    void updateTypeName(QString);//修改标签值


private:
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);
    void createActions();
    void createCentralWindow();
    void createMenus();
    void updateActions();
    void scaleImage(double factor);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void retranslate();
    QStringList loadTypeNameFromFile(QString filePath);
    void displayImageView(QString imageFilePath);

    QWidget *_centralWidget;
    QAction *_fitToWindowAct;
    QHBoxLayout *_horizontalLayout;
    QSplitter *_mainSplitter;
    QTreeView *_fileListView;
    CustomView *_imageView;
    CustomScene *_imageScene = nullptr;
    QDirModel *_fileListModel = nullptr;
    QStringList _filters;
    QString _typeNameFile;
    QStringList _typeNameList;
    QString _languageFile;
    bool _isImageLoaded =  false;
    QString _selectedImageName;
    QLabel *_labelImageInfo, *_labelImageIndex, *_labelCursorPos,*_labelTypeName,*_editLabel;
    QLineEdit *_editImageIndex,*_labelEdit;
    QPointF _cursorPos;
    QSize _imageSize;
    QRect _boxRect;
    QString _boxTypeName;
    QAction *_deleteAct;
    QTranslator _translator;

    QAction *_zhCNAct, *_enUSAct;
    QMenu *_fileMenu;
    QToolBar *_fileToolBar;
    QAction *_goHome;
    QAction *_openAct;
    QAction *_exitAct;
    QMenu *_editMenu;
    QToolBar *_editToolBar;
    QAction *_drawPloy;
    QAction *_drawMagic;
    QAction *_drawBrush;
    QAction *_nextAct;
    QAction *_prevAct;
    QAction *_undoAct;
    QAction *_redoAct;
    QAction *_copyAct;
    QAction *_cutAct;
    QAction *_pasteAct;
    QUndoGroup *_undoGroup;
    QMenu *_viewMenu;
    QToolBar *_viewToolBar;
    QAction *_panAct;
    QAction *_zoomInAct;
    QAction *_zoomOutAct;
    QAction *_actualSizeAct;
    QAction *_fullscreenAct;
    QMenu *_helpMenu;
    QToolBar *_helpToolBar;
    QMenu *_languageMenu;
    QAction *_helpAct;
    QAction *_aboutAct;
    QAction *_exportMask;
    QMessageBox _helpMessageBox, _aboutMessageBox;
    const char *_helpText = "<p>"
                           "Press <b>Right Mouse Button</b> on selected box to change target type.<br />"
                           "<hr />"
                           "<b>Ctrl + D:</b> Draw Box<br />"
                           "<hr />"
                           "<b>Delete Key:</b> Delete Selected Box<br />"
                           "<hr />"
                           "<b>Ctrl + A:</b> Select All Boxes<br />"
                           "<hr />"
                           "<b>Up/Down Arrow Key:</b> Switch images</p>";
    char _aboutText[1024] = {0};// = "<p><b>Image Labeler</b> is based on Qt 5.10.1 and FreeImage 3.18.</p>";

    const QString _trHelpText = tr("<p>"
                           "Press <b>Right Mouse Button</b> on selected box to change target type.<br />"
                           "<hr />"
                           "<b>Ctrl + D:</b> Draw Box<br />"
                           "<hr />"
                           "<b>Delete Key:</b> Delete Selected Box<br />"
                           "<hr />"
                           "<b>Ctrl + A:</b> Select All Boxes<br />"
                           "<hr />"
                           "<b>Up/Down Arrow Key:</b> Switch images</p>");
    const QString _trAboutText = tr("<p><b>Image Labeler 2.1.0</b> is based on Qt 5.10.1 and FreeImage 3.18.</p>");
};
//! [0]

#endif

