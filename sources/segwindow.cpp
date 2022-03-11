/****************************************************************************
**
分割功能主界面
**
****************************************************************************/
#include <QScrollBar>
#include <QImageWriter>
#include <QFileSystemModel>
#include <QStandardPaths>
#include <QDebug>
#include <QMouseEvent>
#include <QFileDialog>
#include <functional>
#include <QtWidgets>
#include<QDebug>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#endif
#include "segwindow.h"
#include "mfile.h"

SegWindow::SegWindow()
{
    setWindowIcon(QIcon(":/images/draw.ico"));
    setWindowTitle(tr("Image Labeler"));
    createActions();
    createCentralWindow();
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
    this->installEventFilter(this);

    _languageFile = ":/languages/zh_CN.qm";
    _translator.load(_languageFile);
    qApp->installTranslator( &_translator );
    this->retranslate();
}

void SegWindow::goHome()
{
    QMessageBox::StandardButton button;
    button=QMessageBox::question(this,tr("back Home"),QString(tr("Are You Sure")),QMessageBox::Yes|QMessageBox::No);
    if(button==QMessageBox::Yes)
    {
        emit goBack();//发送返回主页的信号
    }
}
/**
 * @brief MainWindow::openFolder
 *        Open the specified directory that contains images to be labeled.
 */

void SegWindow::openFolder()
{
    readListFile();
    QString srcImageDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                            "",
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
    if (!srcImageDir.isEmpty()) {
        setWindowTitle(srcImageDir);

        if (_fileListModel){
            delete _fileListModel;
            _fileListModel = nullptr;
            _editAct->setEnabled(false);
            _drawPloy->setEnabled(false);
            _deleteAct->setEnabled(false);
        }

        // init dirmodel
        _filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.tif" << "*.tiff" << "*.bmp" << "*.ppm";
        _fileListModel = new QDirModel(_filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name, this);

        // init treeview and set model
        _fileListView->setRootIsDecorated(0);
        _fileListView->setModel(_fileListModel);
        _fileListView->setRootIndex(_fileListModel->index(srcImageDir));
        _fileListView->setSelectionMode(QAbstractItemView::SingleSelection);

        connect(_fileListView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &SegWindow::onFileSelected);

        // save image names to txt file
        //        saveImageNamesToFile(srcImageDir + "/train.txt");//保存训练文件名


        // set the first image selected.
        QModelIndex index = _fileListModel->index(0, 0, _fileListView->rootIndex());
        _fileListView->setCurrentIndex(index);
        _panAct->setEnabled(true);
        //设置上一张和下一张图片
        int rowCount = _fileListModel->rowCount(_fileListView->rootIndex());
        if (rowCount > 1) {
            _nextAct->setEnabled(true);
            _prevAct->setEnabled(true);
            _exportMask->setEnabled(true);
        }

        _editImageIndex->setValidator(new QIntValidator(1, _fileListModel->rowCount(_fileListView->rootIndex()), this));
        _editImageIndex->setAlignment(Qt::AlignRight);
        _editImageIndex->setHidden(false);
        connect(_editImageIndex, SIGNAL(returnPressed()), this, SLOT(selectFile()));
        _editAct->setEnabled(true);
        _drawPloy->setEnabled(true);
        _deleteAct->setEnabled(true);
    }
    else
        return;


}
//save train.txt
void SegWindow::saveImageNamesToFile(const QString fileName)
{

    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("utf-8"));

    QModelIndex rootIndex = _fileListView->rootIndex();
    int rowCount = _fileListModel->rowCount(rootIndex);

    for (int i = 0; i < rowCount; ++i) {
        QModelIndex index = _fileListModel->index(i, 0, rootIndex);
        QString name = _fileListModel->filePath(index);
        out << name + "\n";
    }

    file.close();
}

QStringList SegWindow::loadTypeNameFromFile(QString filePath)//加载下拉标签框
{
    QStringList typeNameList;
    QFile file(filePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString s = in.readLine();
        if(!(s.simplified().isEmpty())) {
            typeNameList.append(s);
        }
    }
    file.close();

    return typeNameList;
}

void SegWindow::help()
{
    _helpMessageBox.about(this,
                          qApp->translate("SegWindow", "Help"),
                          qApp->translate("SegWindow", _helpText));
}

void SegWindow::about()
{
    sprintf(_aboutText, "<p><b>Image Labeler %d.%d.%d</b> is based on Qt 5.10.1 and FreeImage 3.18.</p>",
            VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
    _aboutMessageBox.about(this,
                           qApp->translate("SegWindow", "About Image Labeler"),
                           qApp->translate("SegWindow", _aboutText));
}

/**
 * @brief SegWindow::createActions add menu actions
 */
void SegWindow::createActions()
{
    // file menu
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _fileToolBar = addToolBar(tr("File"));
    //go back
    _goHome = new QAction(QIcon(":/images/home.png"), tr("&Go Home..."), this);
    _goHome->setShortcuts(QKeySequence::Back);
    _goHome->setStatusTip(tr("Go Home"));
    connect(_goHome, &QAction::triggered, this, &SegWindow::goHome);
    _fileMenu->addAction(_goHome);
    _fileToolBar->addAction(_goHome);
    // open folder
    _openAct = new QAction(QIcon(":/images/folder.png"), tr("&Open Folder..."), this);
    _openAct->setShortcuts(QKeySequence::Open);
    _openAct->setStatusTip(tr("Open an image folder"));
    connect(_openAct, &QAction::triggered, this, &SegWindow::openFolder);
    _fileMenu->addAction(_openAct);
    _fileToolBar->addAction(_openAct);

    menuBar()->addSeparator();

    // edit menu
    _editMenu = menuBar()->addMenu(tr("&Edit"));
    _editToolBar = addToolBar(tr("Edit"));



    // edit type name
    _editAct = new QAction(QIcon(":/images/edit.png"), tr("T&arget Type"), this);
    _editAct->setShortcut(tr("Ctrl+T"));
    _editAct->setStatusTip(tr("Edit Target Type"));
    connect(_editAct, &QAction::triggered, this, &SegWindow::editTypeNameList);
    _editMenu->addAction(_editAct);
    _editToolBar->addAction(_editAct);
    _editAct->setEnabled(false);

    // target type combobox
    _typeNameComboBox = new QComboBox(this);
    _editToolBar->addWidget(_typeNameComboBox);
    _typeNameComboBox->installEventFilter(this);

    // draw ploy points
    _drawPloy = new QAction(QIcon(":/images/ploy.png"), tr("&Draw Ploy"), this);
    _drawPloy->setShortcut(tr("Ctrl+D"));
    _drawPloy->setStatusTip(tr("Draw Ploy"));
    connect(_drawPloy, &QAction::triggered, this, &SegWindow::drawPloyItem);
    _editMenu->addAction(_drawPloy);
    _editToolBar->addAction(_drawPloy);
    _drawPloy->setEnabled(false);
    _drawPloy->setCheckable(true);
    _drawPloy->setChecked(false);
    _editMenu->addSeparator();
    // draw magic rect
    _drawMagic = new QAction(QIcon(":/images/magic.png"), tr("&Draw Magic Box"), this);
    _drawMagic->setStatusTip(tr("Draw Magic Box"));
    connect(_drawMagic, &QAction::toggled, this, &SegWindow::drawPloyItem);
    _editMenu->addAction(_drawMagic);
    _editToolBar->addAction(_drawMagic);
    _drawMagic->setEnabled(false);
    _drawMagic->setCheckable(true);
    _drawMagic->setChecked(false);
    _drawMagic->setVisible(false);//不可见
    _editMenu->addSeparator();

    // draw magic rect
    _drawBrush = new QAction(QIcon(":/images/brush.png"), tr("&Draw brush Box"), this);
    _drawBrush->setShortcut(tr("Ctrl+D"));
    _drawBrush->setStatusTip(tr("Draw brush Box"));
    connect(_drawBrush, &QAction::toggled, this, &SegWindow::drawPloyItem);
    _editMenu->addAction(_drawBrush);
    _editToolBar->addAction(_drawBrush);
    _drawBrush->setEnabled(false);
    _drawBrush->setCheckable(true);
    _drawBrush->setChecked(false);
    _drawBrush->setVisible(false);//不可见

    _editMenu->addSeparator();

    _undoGroup = new QUndoGroup(this);

    // undo
    _undoAct = _undoGroup->createUndoAction(this);
    _undoAct->setIcon(QIcon(":/images/undo.png"));
    _undoAct->setText(tr("&Undo"));
    _undoAct->setStatusTip(tr("Undo"));
    _undoAct->setShortcut(QKeySequence::Undo);
    _editMenu->addAction(_undoAct);
    _editToolBar->addAction(_undoAct);

    // redo
    _redoAct = _undoGroup->createRedoAction(this);
    _redoAct->setIcon(QIcon(":/images/redo.png"));
    _redoAct->setText(tr("&Redo"));
    _redoAct->setStatusTip(tr("Redo"));
    _redoAct->setShortcut(QKeySequence::Redo);
    _editMenu->addAction(_redoAct);
    _editToolBar->addAction(_redoAct);

    _editMenu->addSeparator();


    // copy
    _copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    _copyAct->setStatusTip(tr("Copy"));
    _copyAct->setShortcut(QKeySequence::Copy);
    _copyAct->setEnabled(false);
    _copyAct->setVisible(false);//不可见
    _editMenu->addAction(_copyAct);

    _editToolBar->addAction(_copyAct);

    // cut
    _cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    _cutAct->setStatusTip(tr("Cut"));
    _cutAct->setShortcut(QKeySequence::Cut);
    _cutAct->setEnabled(false);
    _cutAct->setVisible(false);//不可见
    _editMenu->addAction(_cutAct);
    _editToolBar->addAction(_cutAct);

    //Delete boxs
    _deleteAct = new QAction(QIcon(":/images/delete.png"), tr("&Delete"), this);
    _deleteAct->setStatusTip(tr("Delte"));
    _deleteAct->setShortcut(QKeySequence::Delete);
    _deleteAct->setEnabled(false);
    _editMenu->addAction(_deleteAct);
    _editToolBar->addAction(_deleteAct);
    // paste
    _pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    _pasteAct->setStatusTip(tr("Paste"));
    _pasteAct->setShortcut(QKeySequence::Paste);
    _pasteAct->setEnabled(false);
    _pasteAct->setVisible(false);//不可见
    _editMenu->addAction(_pasteAct);
    _editToolBar->addAction(_pasteAct);

    menuBar()->addSeparator();

    // view menu
    _viewMenu = menuBar()->addMenu(tr("&View"));
    _viewToolBar = addToolBar(tr("View"));

    // pan
    _panAct = new QAction(QIcon(":/images/hand.png"), tr("&Pan"), this);
    _panAct->setShortcut(tr("Ctrl+P"));
    _panAct->setStatusTip(tr("Pan Image"));
    _panAct->setCheckable(true);
    _panAct->setChecked(false);
    _panAct->setEnabled(false);
    connect(_panAct, &QAction::toggled, this, &SegWindow::panImage);
    _viewMenu->addAction(_panAct);
    _viewToolBar->addAction(_panAct);
    // fit to window
    _fitToWindowAct = new QAction(QIcon(":/images/fit-to-window.png"), tr("&Fit To Window"), this);
    _fitToWindowAct->setShortcut(tr("Ctrl+F"));
    _fitToWindowAct->setStatusTip(tr("Fit View To Window"));
    connect(_fitToWindowAct, &QAction::triggered, this, &SegWindow::fitViewToWindow);
    _fitToWindowAct->setCheckable(true);
    _fitToWindowAct->setChecked(true);
    _viewMenu->addAction(_fitToWindowAct);
    _viewToolBar->addAction(_fitToWindowAct);

    // actual size
    _actualSizeAct = new QAction(QIcon(":/images/actual-size.png"), tr("&Actual Size"), this);
    _actualSizeAct->setShortcut(tr("Ctrl+R"));
    _actualSizeAct->setStatusTip(tr("Fit View To Actual Size"));
    connect(_actualSizeAct, &QAction::triggered, this, &SegWindow::fitViewToActual);
    _viewMenu->addAction(_actualSizeAct);
    _viewToolBar->addAction(_actualSizeAct);

    _viewMenu->addSeparator();

    // full screen
    _fullscreenAct = new QAction(QIcon(":/images/fullscreen.png"), tr("&Full Screen"), this);
    _fullscreenAct->setShortcut(tr("Alt+Enter"));
    _fullscreenAct->setStatusTip(tr("Full Screen"));
    connect(_fullscreenAct, &QAction::triggered, this, &SegWindow::fullScreen);
    _viewMenu->addAction(_fullscreenAct);
    _viewToolBar->addAction(_fullscreenAct);

    //Prev Image
    // Prev Image
    _prevAct = new QAction(QIcon(":/images/prev.png"), tr("&Prev"), this);
    _prevAct->setStatusTip(tr("Prev"));
    _prevAct->setShortcut(tr("A"));
    _prevAct->setEnabled(false);
    connect(_prevAct, &QAction::triggered, this, &SegWindow::prevImage);
    _viewMenu->addAction(_prevAct);
    _viewToolBar->addAction(_prevAct);
    // Next Image
    _nextAct = new QAction(QIcon(":/images/next.png"), tr("&Next"), this);
    _nextAct->setStatusTip(tr("Next"));
    _nextAct->setShortcut(tr("D"));
    _nextAct->setEnabled(false);
    connect(_nextAct, &QAction::triggered, this, &SegWindow::nextImage);
    _viewMenu->addAction(_nextAct);
    _viewToolBar->addAction(_nextAct);

    _viewMenu->addSeparator();//添加分割线


    // export Seg Image
    _exportMask = new QAction(QIcon(":/images/export.png"), tr("&Export Image"), this);
    _exportMask->setStatusTip(tr("Export"));
    _exportMask->setEnabled(false);
    _viewMenu->addAction(_exportMask);
    _viewToolBar->addAction(_exportMask);



    // help menu
    _helpMenu = menuBar()->addMenu(tr("&Help"));
    _helpToolBar = addToolBar(tr("Help"));

    // language
    _languageMenu = _helpMenu->addMenu(QIcon(":/images/language.png"), tr("&Language"));
    _languageMenu->setStatusTip(tr("Select language"));

    // Chinese
    _zhCNAct = _languageMenu->addAction(QIcon(":/images/zh_CN.png"),
                                        tr("&Chinese"));
    _languageMenu->addAction(_zhCNAct);
    //    _helpToolBar->addAction(_zhCNAct);
    connect(_zhCNAct, &QAction::triggered, this, &SegWindow::switchLanguage);

    // English
    _enUSAct = _languageMenu->addAction(QIcon(":/images/en_US.png"),
                                        tr("&English"));
    _languageMenu->addAction(_enUSAct);
    //    _helpToolBar->addAction(_enUSAct);
    connect(_enUSAct, &QAction::triggered, this, &SegWindow::switchLanguage);

    // help
    _helpAct = _helpMenu->addAction(QIcon(":/images/help.png"),
                                    tr("&Help"), this, &SegWindow::help);
    _helpAct->setStatusTip(tr("Help"));
    _helpAct->setShortcut(QKeySequence::HelpContents);
    //    _helpToolBar->addAction(_helpAct);

    // about
    _aboutAct = _helpMenu->addAction(QIcon(":/images/about.png"),
                                     tr("&About"), this, &SegWindow::about);
    _aboutAct->setStatusTip(tr("About Image Labeler"));
    //    _helpToolBar->addAction(_aboutAct);
}

void SegWindow::switchLanguage()
{
    if (this->sender() == _zhCNAct) {
        _languageFile = ":/languages/zh_CN.qm";
    } else if (this->sender() == _enUSAct) {
        _languageFile = ":/languages/en_US.qm";
    }
    _translator.load(_languageFile);
    qApp->installTranslator( &_translator );
    this->retranslate();
}

void SegWindow::retranslate()
{
    // file menu
    _fileMenu->setTitle(tr("&File"));
    //    fileToolBar->setTitle(tr("File"));

    // open folder
    _openAct->setText(tr("&Open Folder..."));
    _openAct->setStatusTip(tr("Open an image folder"));

    //    // quit
    //    _exitAct->setText(tr("E&xit"));
    //    _exitAct->setShortcut(tr("Ctrl+Q"));
    //    _exitAct->setStatusTip(tr("Exit the application"));

    // edit menu
    _editMenu->setTitle(tr("&Edit"));

    // draw box rect
    _drawPloy->setText(tr("&Draw Ploy"));
    _drawPloy->setShortcut(tr("Ctrl+E"));//添加快捷键操作
    _drawPloy->setStatusTip(tr("Draw Ploy"));

    // edit target type
    _editAct ->setText(tr("T&arget Type"));
    _editAct->setShortcut(tr("Ctrl+T"));
    _editAct->setStatusTip(tr("Edit Target Type"));

    // undo
    _undoAct->setText(tr("&Undo"));
    _undoAct->setStatusTip(tr("Undo"));

    // redo
    _redoAct->setText(tr("&Redo"));
    _redoAct->setStatusTip(tr("Redo"));

    // copy
    _copyAct->setText(tr("&Copy"));
    _copyAct->setStatusTip(tr("Copy"));

    // cut
    _cutAct->setText(tr("Cu&t"));
    _cutAct->setStatusTip(tr("Cut"));

    // paste
    _pasteAct->setText(tr("&Paste"));
    _pasteAct->setStatusTip(tr("Paste"));

    // view menu
    _viewMenu->setTitle(tr("&View"));
    //    viewToolBar->setTitle(tr("View"));

    // pan
    _panAct->setText(tr("&Pan"));
    _panAct->setShortcut(tr("Ctrl+P"));
    _panAct->setStatusTip(tr("Pan Image"));
    // zoom in
    //    _zoomInAct->setText(tr("Zoom &In"));
    //    _zoomInAct->setShortcut(QKeySequence::ZoomIn);
    //    _zoomInAct->setStatusTip(tr("Zoom In Image"));
    // zoom out
    //    _zoomOutAct->setText(tr("Zoom &Out"));
    //    _zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    //    _zoomOutAct->setStatusTip(tr("Zoom Out Image"));
    // fit to window
    _fitToWindowAct->setText(tr("&Fit To Window"));
    _fitToWindowAct->setShortcut(tr("Ctrl+F"));
    _fitToWindowAct->setStatusTip(tr("Fit View To Window"));

    // actual size
    _actualSizeAct->setText(tr("&Actual Size"));
    _actualSizeAct->setShortcut(tr("Ctrl+R"));
    _actualSizeAct->setStatusTip(tr("Fit View To Actual Size"));

    // full screen
    _fullscreenAct->setText(tr("&Full Screen"));
    _fullscreenAct->setShortcut(tr("Alt+Enter"));
    _fullscreenAct->setStatusTip(tr("Full Screen"));
    // help menu
    _helpMenu->setTitle(tr("&Help"));
    //    helpToolBar = addToolBar(tr("Help"));

    // language
    _languageMenu->setTitle(tr("&Language"));
    _languageMenu->setStatusTip(tr("Select language"));

    // Chinese
    _zhCNAct->setText(tr("&Chinese"));

    // English
    _enUSAct->setText(tr("&English"));

    // help
    _helpAct->setText(tr("&Help"));
    _helpAct->setStatusTip(tr("Help"));

    // about
    _aboutAct->setText(tr("&About"));
    _aboutAct->setToolTip(tr("About Image Labeler"));

    // status bar
    if (!_imageSize.isEmpty())
        _labelImageInfo->setText(QString(tr("Image: %1 x %2"))
                                 .arg(_imageSize.width())
                                 .arg(_imageSize.height())
                                 .toUtf8());
    if (!_boxRect.isNull())
        _labelTypeName->setText(QString(tr("currenttype-%5"))
                                .arg(_boxTypeName)
                                .toUtf8());
    if (!_cursorPos.isNull())
        _labelCursorPos->setText(QString(tr("Cursor: %1, %2"))
                                 .arg((int)_cursorPos.x())
                                 .arg((int)_cursorPos.y())
                                 .toUtf8());
}

void SegWindow::createCentralWindow()
{
    _centralWidget = new QWidget(this);
    _horizontalLayout = new QHBoxLayout(_centralWidget);
    _fileListView = new QTreeView(this);
    //    _fileListView->setSortingEnabled(true);

    _imageView = new CustomView(this);
    _imageView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    _mainSplitter = new QSplitter(Qt::Horizontal, _centralWidget);
    _mainSplitter->addWidget(_fileListView);
    _mainSplitter->addWidget(_imageView);
    _mainSplitter->setStretchFactor(0, 2);
    _mainSplitter->setStretchFactor(1, 8);
    _mainSplitter->setOpaqueResize(false);

    _horizontalLayout->addWidget(_mainSplitter);
    _centralWidget->setLayout(_horizontalLayout);
    this->setCentralWidget(_centralWidget);

    _labelImageIndex = new QLabel();
    _editImageIndex = new QLineEdit();
    _labelImageInfo = new QLabel();
    _labelCursorPos = new QLabel();
    _labelTypeName=new QLabel();
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_editImageIndex, 1);
    _editImageIndex->setHidden(true);
    this->statusBar()->addPermanentWidget(_labelImageIndex, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_labelImageInfo, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_labelCursorPos, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_labelTypeName, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
}

void SegWindow::closeEvent(QCloseEvent *event)
{
    if (_imageScene) {
        delete _imageScene;
    }
}

void SegWindow::updateActions()
{
    _zoomInAct->setEnabled(!_fitToWindowAct->isChecked());
    _zoomOutAct->setEnabled(!_fitToWindowAct->isChecked());
}

void SegWindow::selectFile()
{
    int row = _editImageIndex->text().toInt();
    QModelIndex index = _fileListModel->index(row-1, 0, _fileListView->rootIndex());
    _fileListView->setCurrentIndex(index);
}

void SegWindow::onFileSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    QModelIndex index = selected.indexes().first();
    displayImageView(_fileListModel->filePath(index));

    _editImageIndex->setText(QString("%1")
                             .arg(index.row()+1));
    _labelImageIndex->setText(QString("/%1")
                              .arg(_fileListModel->rowCount(_fileListView->rootIndex()))
                              .toUtf8());
    _selectedImageName = _fileListModel->fileName(index);
    _labelImageInfo->setText(QString(tr("Image: %1 Size: %2 x %3"))
                             .arg(_selectedImageName)
                             .arg(_imageSize.width())
                             .arg(_imageSize.height())
                             .toUtf8());
}

void SegWindow::displayImageView(QString imageFilePath)
{

    if (_imageScene) {
        delete _imageScene;
    }
   

    _imageScene = new CustomScene(this);
    _imageScene->setTypeNameList(_typeNameList);
    _imageScene->setTypeName(_typeNameComboBox->currentText());
    _imageScene->installEventFilter(this);
    _imageScene->setSegTask(true);
    connect(_imageScene, SIGNAL(cursorMoved(QPointF)), this, SLOT(updateLabelCursorPos(QPointF)));
    connect(_imageScene, SIGNAL(boxSelected(QRect, QString)), this, SLOT(updateBoxInfo(QRect, QString)));
    connect(_imageScene, SIGNAL(imageLoaded(QSize)), this, SLOT(updateLabelImageSize(QSize)));
    connect(_imageScene, SIGNAL(typeChange(QString)), this, SLOT(updateTypeName(QString)));//改变下拉框的值
    connect(_typeNameComboBox, SIGNAL(activated(QString)), _imageScene, SLOT(changePointTypeName(QString)));//改变下拉框
    connect(_deleteAct, SIGNAL(triggered()), _imageScene, SLOT(delete_ploy()));//删除事件
    connect(_copyAct, SIGNAL(triggered()), _imageScene, SLOT(copy()));
    connect(_pasteAct, SIGNAL(triggered()), _imageScene, SLOT(paste()));
    connect(_cutAct, SIGNAL(triggered()), _imageScene, SLOT(cut()));
    connect(_exportMask, SIGNAL(triggered()), _imageScene, SLOT(exportMaskPic()));
    connect(_imageScene, SIGNAL(selectionChanged()), this, SLOT(updateCopyCutActions()));
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(updatePasteAction()));
    qDebug()<<"the displayImageView is............."<<_imageScene->selectedItems().count();
    _copyAct->setEnabled(false);
    _pasteAct->setEnabled(false);
    _cutAct->setEnabled(false);
    _imageScene->loadImage(imageFilePath);
    _isImageLoaded = true;

    // init box info on the status bar
    int boxCount = 0;
    foreach (QGraphicsItem *item, _imageScene->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            PointItem *b = qgraphicsitem_cast<PointItem *>(item);
            b->setSelected(true);
            boxCount++;
            break;
        }
    }
    //    if (boxCount == 0)
    //        updateBoxInfo(QRect(), QString());

    _undoGroup->addStack(_imageScene->undoStack());
    _undoGroup->setActiveStack(_imageScene ? _imageScene->undoStack() : 0);

    _imageView->setScene(_imageScene);

    _panAct->setChecked(false);
    if (_drawPloy->isChecked()) {
        drawPloyItem(true);
    }
    _fitToWindowAct->setChecked(true);
    fitViewToWindow();

    // init drawing status from _drawPloy
    _imageScene->drawPloyItem(_drawPloy->isChecked());
}

void SegWindow::showEvent(QShowEvent* event)
{
    fitViewToWindow();
}

void SegWindow::resizeEvent(QResizeEvent* event)
{
    fitViewToWindow();
}

bool SegWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _imageScene || obj == _typeNameComboBox || obj == this) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            // use Key_Down and Key_Up received by _viewScene and SegWindow to select image in fileListView
            if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up ) {
                int rowCount = _fileListModel->rowCount(_fileListView->rootIndex());
                int currentRow = _fileListView->currentIndex().row();
                if (keyEvent->key() == Qt::Key_Down) {
                    currentRow = qMin(rowCount-1, currentRow+1);
                } else if (keyEvent->key() == Qt::Key_Up) {
                    currentRow = qMax(0, currentRow-1);
                }
                QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
                _fileListView->setCurrentIndex(index);
                return true;
            }
        }
        return false;
    } else {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}

void SegWindow::drawPloyItem(bool checked)
{
    int count=_imageScene->selectedItems().count();
    qDebug()<<"draw Ploy Item ........"<<count;
    if (!_isImageLoaded)
        return;

    if (_panAct->isChecked()) {
        _panAct->setChecked(false);
    }
    _deleteAct->setEnabled(!checked);
    if(count<1) _deleteAct->setChecked(false);
    //    else _deleteAct->setEnabled(!checked);
    _imageView->drawPloyItem(checked);
    _imageScene->drawPloyItem(checked);
}
/**
 * @brief SegWindow::panImage
 * @param checked
 * 手掌移动
 */
void SegWindow::panImage(bool checked)
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    if (_drawPloy->isChecked())
        _drawPloy->setChecked(false);//这里有个bug
    _deleteAct->setEnabled(!checked);
    _imageView->setPointTask(true);//设置为语义分割任务
    _imageView->panImage(checked);
    _imageScene->panImage(checked);
}

void SegWindow::editTypeNameList()
{
    readListFile();
    //TypeEditDialog* d = new TypeEditDialog(this, _typeNameFile, &_translator);
    //int r = d->exec();
    //if (r == QDialog::Accepted) {
    //    delete d;
    //}
}

void SegWindow::updateCopyCutActions()
{
    bool op = _imageScene->selectedItems().count() > 0;

    _copyAct->setEnabled(op);
    _cutAct->setEnabled(op);
    //    _deleteAct->setEnabled(op);
}

void SegWindow::updatePasteAction()
{
    _pasteAct->setEnabled(true);
}

//add by asf
void SegWindow::readListFile()
{

    QString textPath = QFileDialog::getOpenFileName(
                this, tr("open label file"),
                "./", tr("*.names"));

    if (!textPath.isEmpty())
    {
        _typeNameList.clear();
        if (!mFile::ReadFileGetLineContent(textPath, _typeNameList))
        {
            qDebug() << "__func__:"<<__func__<<"	read file error ";
        }
        if (_typeNameList.count()<= 0)
        {
            QMessageBox::critical(0 , "Alert" , "标签文件内容不能为空", QMessageBox::Ok | QMessageBox::Default , QMessageBox::Cancel | QMessageBox::Escape , 	0 );
            return;
        }
        foreach(QString line, _typeNameList)
            qDebug() << line;
        _typeNameComboBox->clear();
        _typeNameComboBox->addItems(_typeNameList);
        _typeNameComboBox->setCurrentIndex(0);
        _typeNameComboBox->setMinimumWidth(120);
        QFont font;
        font.setPixelSize(20);
        _typeNameComboBox->setFont(font);
        _drawPloy->setEnabled(true);
        int currentRow=_fileListView->currentIndex().row();
        if(currentRow>=0){
            QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
            displayImageView(_fileListModel->filePath(index));
        }

    }else{
        return;
    }
}

void SegWindow::wheelEvent(QWheelEvent *event)//鼠标滚轮事件
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    qreal newZoom = 1 + (event->delta() / 200.0) * 0.05;
    _imageView->scale(newZoom, newZoom);
}

void SegWindow::fitViewToWindow()
{
    if (!_isImageLoaded)
        return;
    if (!_fitToWindowAct->isChecked())
        return;
    if (_panAct->isChecked())
        _panAct->setChecked(false);

    _imageView->fitInView(_imageView->sceneRect(), Qt::KeepAspectRatio);
}

void SegWindow::fitViewToActual()
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    _imageView->fitInView(_imageView->rect(), Qt::KeepAspectRatio); // please check this issue.
}
//next Image
void SegWindow::nextImage()
{
    int rowCount=_fileListModel->rowCount(_fileListView->rootIndex());
    int currentRow=_fileListView->currentIndex().row();
    currentRow=qMin(rowCount-1,currentRow+1);
    QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
    _fileListView->rootIndex();
    _fileListView->setCurrentIndex(index);
}
//prev Image
void SegWindow::prevImage()
{
    int currentRow=_fileListView->currentIndex().row();
    currentRow=qMax(0,currentRow-1);
    QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
    _fileListView->rootIndex();
    _fileListView->setCurrentIndex(index);
}
//full Screen
void SegWindow::fullScreen()
{
    auto makeFullscreen = !isFullScreen();

    _fileListView->setVisible(!makeFullscreen);
    setWindowState(makeFullscreen ? Qt::WindowFullScreen : Qt::WindowMaximized);
}

void SegWindow::updateLabelImageSize(QSize imageSize)
{
    _imageSize = imageSize;
    _labelImageInfo->setText(QString(tr("Image: %1 Size: %2 x %3"))
                             .arg(_selectedImageName)
                             .arg(_imageSize.width())
                             .arg(_imageSize.height())
                             .toUtf8());
}

void SegWindow::updateBoxInfo(QRect rect, QString typeName)
{
    if (typeName.isNull() || rect.isNull())
    {
        //        _labelBoxInfo->setText(QString(tr("Box: x- , y- , w- , h- , type: "))
        //                               .toUtf8());
        return;
    }
    _boxRect = rect;
    _boxTypeName = typeName;
    //    _labelBoxInfo->setText(QString(tr("Box: x-%1, y-%2, w-%3, h-%4, type: %5"))
    //                           .arg(_boxRect.left())
    //                           .arg(_boxRect.top())
    //                           .arg(_boxRect.width())
    //                           .arg(_boxRect.height())
    //                           .arg(_boxTypeName)
    //                           .toUtf8());
    _typeNameComboBox->setCurrentText(_boxTypeName);
}

void SegWindow::updateLabelCursorPos(QPointF cursorPos)
{
    _cursorPos = cursorPos;
    _labelCursorPos->setText(QString(tr("Cursor: %1, %2"))
                             .arg((int)_cursorPos.x())
                             .arg((int)_cursorPos.y())
                             .toUtf8());
}
/**
 * @brief updateTypeName
 * @param typeName
 * 修改当前标签值
 */
void SegWindow::updateTypeName(QString  typeName)
{
    _typeNameComboBox->setCurrentText(typeName);
}
