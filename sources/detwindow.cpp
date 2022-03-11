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

#include "detwindow.h"
#include "mfile.h"

DetWindow::DetWindow()
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

void DetWindow::goHome()
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

void DetWindow::openFolder()
{
    readListFile();
    emit isBoxTask();
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
            _drawAct->setEnabled(true);
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
                this, &DetWindow::onFileSelected);

        // save image names to txt file
        //saveImageNamesToFile(srcImageDir + "/train.txt");//保存训练文件名

        // add type name on combobox
//        _typeNameComboBox->clear();
        //_typeNameComboBox->addItems(_typeNameList);

        // set the first image selected.
        QModelIndex index = _fileListModel->index(0, 0, _fileListView->rootIndex());
        _fileListView->setCurrentIndex(index);
//        _drawAct->setEnabled(true);
        _panAct->setEnabled(true);
        //设置上一张和下一张图片
        int rowCount = _fileListModel->rowCount(_fileListView->rootIndex());
        if (rowCount > 1) {
            _nextAct->setEnabled(true);
            _prevAct->setEnabled(true);
            _exportVOC->setEnabled(true);
            augMent->setEnabled(true);
        }

        _editImageIndex->setValidator(new QIntValidator(1, _fileListModel->rowCount(_fileListView->rootIndex()), this));
        _editImageIndex->setAlignment(Qt::AlignRight);
        _editImageIndex->setHidden(false);
        connect(_editImageIndex, SIGNAL(returnPressed()), this, SLOT(selectFile()));
        _editAct->setEnabled(true);
    }
    else
        return;


}
//save train.txt
void DetWindow::saveImageNamesToFile(const QString fileName)
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

QStringList DetWindow::loadTypeNameFromFile(QString filePath)//加载下拉框
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

void DetWindow::help()
{
    _helpMessageBox.about(this,
                          qApp->translate("DetWindow", "Help"),
                          qApp->translate("DetWindow", _helpText));
}

void DetWindow::about()
{
    sprintf(_aboutText, "<p><b>Image Labeler %d.%d.%d</b> is based on Qt 5.10.1 and FreeImage 3.18.</p>",
            VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
    _aboutMessageBox.about(this,
                           qApp->translate("DetWindow", "About Image Labeler"),
                           qApp->translate("DetWindow", _aboutText));
}

/**
 * @brief DetWindow::createActions add menu actions
 */
void DetWindow::createActions()
{
    // file menu
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _fileToolBar = addToolBar(tr("File"));
    //go back
    _goHome = new QAction(QIcon(":/images/home.png"), tr("&Go Home..."), this);
    _goHome->setShortcuts(QKeySequence::Back);
    _goHome->setStatusTip(tr("Go Home"));
    connect(_goHome, &QAction::triggered, this, &DetWindow::goHome);
    _fileMenu->addAction(_goHome);
    _fileToolBar->addAction(_goHome);
    // open folder
    _openAct = new QAction(QIcon(":/images/folder.png"), tr("&Open Folder..."), this);
    _openAct->setShortcuts(QKeySequence::Open);
    _openAct->setStatusTip(tr("Open an image folder"));
    connect(_openAct, &QAction::triggered, this, &DetWindow::openFolder);
    _fileMenu->addAction(_openAct);
    _fileToolBar->addAction(_openAct);

    //    // quit
    //    _exitAct = _fileMenu->addAction(QIcon(":/images/quit.png"), tr("E&xit"), qApp, &QApplication::closeAllWindows);
    //    _exitAct->setShortcut(tr("Ctrl+Q"));
    //    _exitAct->setStatusTip(tr("Exit the application"));
    //    _fileMenu->addAction(_exitAct);
    //    _fileToolBar->addAction(_exitAct);

    menuBar()->addSeparator();

    // edit menu
    _editMenu = menuBar()->addMenu(tr("&Edit"));
    _editToolBar = addToolBar(tr("Edit"));



    // edit type name
    _editAct = new QAction(QIcon(":/images/edit.png"), tr("T&arget Type"), this);
    _editAct->setShortcut(tr("Ctrl+T"));
    _editAct->setStatusTip(tr("Edit Target Type"));
    connect(_editAct, &QAction::triggered, this, &DetWindow::editTypeNameList);
    _editMenu->addAction(_editAct);
    _editToolBar->addAction(_editAct);
    _editAct->setEnabled(false);

    // target type combobox
    _typeNameComboBox = new QComboBox(this);
    _editToolBar->addWidget(_typeNameComboBox);
    _typeNameComboBox->installEventFilter(this);

    // draw box rect
    _drawAct = new QAction(QIcon(":/images/draw.png"), tr("&Draw Box"), this);
    _drawAct->setShortcut(tr("Ctrl+D"));
    _drawAct->setStatusTip(tr("Draw Box"));
    connect(_drawAct, &QAction::toggled, this, &DetWindow::drawBoxItem);
    _editMenu->addAction(_drawAct);
    _editToolBar->addAction(_drawAct);
    _drawAct->setEnabled(true);
    _drawAct->setCheckable(true);
    _drawAct->setChecked(false);
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
    _editMenu->addAction(_copyAct);
    _editToolBar->addAction(_copyAct);

    // cut
    _cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    _cutAct->setStatusTip(tr("Cut"));
    _cutAct->setShortcut(QKeySequence::Cut);
    _cutAct->setEnabled(false);
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
    connect(_panAct, &QAction::toggled, this, &DetWindow::panImage);
    _viewMenu->addAction(_panAct);
    _viewToolBar->addAction(_panAct);

    // zoom in
//    _zoomInAct = new QAction(QIcon(":/images/zoom-in.png"), tr("Zoom &In"), this);
//    _zoomInAct->setShortcut(QKeySequence::ZoomIn);
//    _zoomInAct->setStatusTip(tr("Zoom In Image"));
//    connect(_zoomInAct, &QAction::triggered, this, &MainWindow::zoomIn);
    //缩小放大功能
    //    _viewMenu->addAction(_zoomInAct);
    //    _viewToolBar->addAction(_zoomInAct);

    // zoom out
//    _zoomOutAct = new QAction(QIcon(":/images/zoom-out.png"), tr("Zoom &Out"), this);
//    _zoomOutAct->setShortcut(QKeySequence::ZoomOut);
//    _zoomOutAct->setStatusTip(tr("Zoom Out Image"));
//    connect(_zoomOutAct, &QAction::triggered, this, &MainWindow::zoomOut);
    //缩小放大功能
    //    _viewMenu->addAction(_zoomOutAct);
    //    _viewToolBar->addAction(_zoomOutAct);

    // fit to window
    _fitToWindowAct = new QAction(QIcon(":/images/fit-to-window.png"), tr("&Fit To Window"), this);
    _fitToWindowAct->setShortcut(tr("Ctrl+F"));
    _fitToWindowAct->setStatusTip(tr("Fit View To Window"));
    connect(_fitToWindowAct, &QAction::triggered, this, &DetWindow::fitViewToWindow);
    _fitToWindowAct->setCheckable(true);
    _fitToWindowAct->setChecked(true);
    _viewMenu->addAction(_fitToWindowAct);
    _viewToolBar->addAction(_fitToWindowAct);

    // actual size
    _actualSizeAct = new QAction(QIcon(":/images/actual-size.png"), tr("&Actual Size"), this);
    _actualSizeAct->setShortcut(tr("Ctrl+R"));
    _actualSizeAct->setStatusTip(tr("Fit View To Actual Size"));
    connect(_actualSizeAct, &QAction::triggered, this, &DetWindow::fitViewToActual);
    _viewMenu->addAction(_actualSizeAct);
    _viewToolBar->addAction(_actualSizeAct);

    _viewMenu->addSeparator();

    // full screen
    _fullscreenAct = new QAction(QIcon(":/images/fullscreen.png"), tr("&Full Screen"), this);
    _fullscreenAct->setShortcut(tr("Alt+Enter"));
    _fullscreenAct->setStatusTip(tr("Full Screen"));
    connect(_fullscreenAct, &QAction::triggered, this, &DetWindow::fullScreen);
    _viewMenu->addAction(_fullscreenAct);
    _viewToolBar->addAction(_fullscreenAct);

    //Prev Image
    // Prev Image
    _prevAct = new QAction(QIcon(":/images/prev.png"), tr("&Prev"), this);
    _prevAct->setStatusTip(tr("Prev"));
    _prevAct->setShortcut(tr("A"));
    _prevAct->setEnabled(false);
    connect(_prevAct, &QAction::triggered, this, &DetWindow::prevImage);
    _viewMenu->addAction(_prevAct);
    _viewToolBar->addAction(_prevAct);
    // Next Image
    _nextAct = new QAction(QIcon(":/images/next.png"), tr("&Next"), this);
    _nextAct->setStatusTip(tr("Next"));
    _nextAct->setShortcut(tr("D"));
    _nextAct->setEnabled(false);
    connect(_nextAct, &QAction::triggered, this, &DetWindow::nextImage);
    _viewMenu->addAction(_nextAct);
    _viewToolBar->addAction(_nextAct);
    //图片增强选项
    augMent=new QAction(QIcon(":/images/augment.png"), tr("&Augment Image"), this);
    augMent->setStatusTip(tr("augment"));
    augMent->setEnabled(false);
    _viewMenu->addAction(augMent);
    _viewToolBar->addAction(augMent);
    // export VOC annotation
    _exportVOC = new QAction(QIcon(":/images/editploy.png"), tr("&Export Image"), this);
    _exportVOC->setStatusTip(tr("Export"));
    _exportVOC->setVisible(true);
    _exportVOC->setEnabled(true);
    _viewMenu->addAction(_exportVOC);
    _viewToolBar->addAction(_exportVOC);

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
    connect(_zhCNAct, &QAction::triggered, this, &DetWindow::switchLanguage);

    // English
    _enUSAct = _languageMenu->addAction(QIcon(":/images/en_US.png"),
                                        tr("&English"));
    _languageMenu->addAction(_enUSAct);
    //    _helpToolBar->addAction(_enUSAct);
    connect(_enUSAct, &QAction::triggered, this, &DetWindow::switchLanguage);

    // help
    _helpAct = _helpMenu->addAction(QIcon(":/images/help.png"),
                                    tr("&Help"), this, &DetWindow::help);
    _helpAct->setStatusTip(tr("Help"));
    _helpAct->setShortcut(QKeySequence::HelpContents);
    //    _helpToolBar->addAction(_helpAct);

    // about
    _aboutAct = _helpMenu->addAction(QIcon(":/images/about.png"),
                                     tr("&About"), this, &DetWindow::about);
    _aboutAct->setStatusTip(tr("About Image Labeler"));
    //    _helpToolBar->addAction(_aboutAct);
}

void DetWindow::switchLanguage()
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

void DetWindow::retranslate()
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
    _drawAct->setText(tr("&Draw Box"));
    _drawAct->setShortcut(tr("Ctrl+E"));//添加快捷键操作
    _drawAct->setStatusTip(tr("Draw Box"));

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
        _labelBoxInfo->setText(QString(tr("Box: x-%1, y-%2, w-%3, h-%4, type-%5"))
                               .arg(_boxRect.left())
                               .arg(_boxRect.top())
                               .arg(_boxRect.width())
                               .arg(_boxRect.height())
                               .arg(_boxTypeName)
                               .toUtf8());
    if (!_cursorPos.isNull())
        _labelCursorPos->setText(QString(tr("Cursor: %1, %2"))
                                 .arg((int)_cursorPos.x())
                                 .arg((int)_cursorPos.y())
                                 .toUtf8());
}

void DetWindow::createCentralWindow()
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
    _labelBoxInfo = new QLabel();

    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_editImageIndex, 1);
    _editImageIndex->setHidden(true);
    this->statusBar()->addPermanentWidget(_labelImageIndex, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_labelImageInfo, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_labelCursorPos, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_labelBoxInfo, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
}

void DetWindow::closeEvent(QCloseEvent *event)
{
    if (_imageScene) {
        delete _imageScene;
    }
}

void DetWindow::updateActions()
{
    _zoomInAct->setEnabled(!_fitToWindowAct->isChecked());
    _zoomOutAct->setEnabled(!_fitToWindowAct->isChecked());
}

void DetWindow::selectFile()
{
    int row = _editImageIndex->text().toInt();
    QModelIndex index = _fileListModel->index(row-1, 0, _fileListView->rootIndex());
    _fileListView->setCurrentIndex(index);
}

void DetWindow::onFileSelected(const QItemSelection& selected, const QItemSelection& deselected)
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

void DetWindow::displayImageView(QString imageFilePath)
{
    if (_imageScene) {
        delete _imageScene;
    }
    _imageScene = new CustomScene(this);

    _imageScene->setTypeNameList(_typeNameList);
    _imageScene->setTypeName(_typeNameComboBox->currentText());
    _imageScene->setBoxTask(true);
    _imageScene->installEventFilter(this);
    connect(_imageScene, SIGNAL(cursorMoved(QPointF)), this, SLOT(updateLabelCursorPos(QPointF)));
    connect(_imageScene, SIGNAL(boxSelected(QRect, QString)), this, SLOT(updateBoxInfo(QRect, QString)));
    connect(_imageScene, SIGNAL(imageLoaded(QSize)), this, SLOT(updateLabelImageSize(QSize)));
    connect(_typeNameComboBox, SIGNAL(activated(QString)), _imageScene, SLOT(changeBoxTypeName(QString)));
    connect(_exportVOC, SIGNAL(triggered()), _imageScene, SLOT(saveBoxItemsToVOC()));
    connect(augMent, SIGNAL(triggered()), _imageScene, SLOT(augMent()));
    connect(_copyAct, SIGNAL(triggered()), _imageScene, SLOT(copy()));
    connect(_pasteAct, SIGNAL(triggered()), _imageScene, SLOT(paste()));
    connect(_cutAct, SIGNAL(triggered()), _imageScene, SLOT(cut()));
    connect(_deleteAct, SIGNAL(triggered()), _imageScene, SLOT(delete_box()));
    connect(_imageScene, SIGNAL(selectionChanged()), this, SLOT(updateCopyCutActions()));
    //    connect(QApplication::clipboard(), SIGNAL(dataChanged()), _imageScene, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(updatePasteAction()));

    _copyAct->setEnabled(false);
    _pasteAct->setEnabled(false);
    _cutAct->setEnabled(false);
    _imageScene->loadImage(imageFilePath);
    _isImageLoaded = true;

    // init box info on the status bar
    int boxCount = 0;
    foreach (QGraphicsItem *item, _imageScene->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            BoxItem *b = qgraphicsitem_cast<BoxItem *>(item);
            b->setSelected(true);
            boxCount++;
            break;
        }
    }
    if (boxCount == 0)
        updateBoxInfo(QRect(), QString());

    _undoGroup->addStack(_imageScene->undoStack());
    _undoGroup->setActiveStack(_imageScene ? _imageScene->undoStack() : 0);

    _imageView->setScene(_imageScene);

    _panAct->setChecked(false);
    if (_drawAct->isChecked()) {
        drawBoxItem(true);
    }
    _fitToWindowAct->setChecked(true);
    fitViewToWindow();

    // init drawing status from _drawAct
    _imageScene->drawBoxItem(_drawAct->isChecked());
}

void DetWindow::showEvent(QShowEvent* event)
{
    fitViewToWindow();
}

void DetWindow::resizeEvent(QResizeEvent* event)
{
    fitViewToWindow();
}

bool DetWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _imageScene || obj == _typeNameComboBox || obj == this) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            // use Key_Down and Key_Up received by _viewScene and DetWindow to select image in fileListView
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

void DetWindow::drawBoxItem(bool checked)
{
    if (!_isImageLoaded)
        return;

    if (_panAct->isChecked()) {
        _panAct->setChecked(false);
    }
    _imageView->drawBoxItem(checked);
    _imageScene->drawBoxItem(checked);
}

void DetWindow::panImage(bool checked)
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    if (_drawAct->isChecked())
        _drawAct->setChecked(false);

    _imageView->panImage(checked);
    _imageScene->panImage(checked);
}

void DetWindow::editTypeNameList()
{
    readListFile();
    //TypeEditDialog* d = new TypeEditDialog(this, _typeNameFile, &_translator);
    //int r = d->exec();
    //if (r == QDialog::Accepted) {
    //    delete d;
    //}
}

void DetWindow::updateCopyCutActions()
{
    bool op = _imageScene->selectedItems().count() > 0;

    _copyAct->setEnabled(op);
    _cutAct->setEnabled(op);
    _deleteAct->setEnabled(op);
}

void DetWindow::updatePasteAction()
{
    _pasteAct->setEnabled(true);
}

//add by asf
void DetWindow::readListFile()
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
//        _typeNameComboBox->setStyleSheet("QComboBox{border-radius:3px;color:black;background:white;padding-left:5px;padding-right:12px;border:1px solid #b5b5b5;}\
//                                                   QComboBox:hover{border:1px solid gray;}\
//                                                   QComboBox::down-arrow{margin-right:10px;width:12px;height:8px;border-image:url(:/TestComboBox/Resources/ComboBoxDownArrow.png);}\
//                                                   QComboBox::down-arrow:on{width:12px;height:8px;border-image:url(:/TestComboBox/Resources/ComboBoxUpArrow.png);}\
//                                                   QComboBox::drop-down{background:transparent;}");
        QFont font;
        font.setPixelSize(20);
        _typeNameComboBox->setFont(font);
        _drawAct->setEnabled(true);
        int currentRow=_fileListView->currentIndex().row();
        if(currentRow>=0){
            QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
            displayImageView(_fileListModel->filePath(index));
        }

    }else{
        return;
    }
}

void DetWindow::zoomIn()
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    _imageView->scale(1.2, 1.2);
}

void DetWindow::zoomOut()
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    _imageView->scale(0.8, 0.8);
}

void DetWindow::wheelEvent(QWheelEvent *event)//鼠标滚轮事件
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    qreal newZoom = 1 + (event->delta() / 120.0) * 0.05;
    _imageView->scale(newZoom, newZoom);
}

void DetWindow::fitViewToWindow()
{
    if (!_isImageLoaded)
        return;
    if (!_fitToWindowAct->isChecked())
        return;
    if (_panAct->isChecked())
        _panAct->setChecked(false);
    _imageView->scale(1.00, 1.00);
//    _imageView->fitInView(_imageView->sceneRect(), Qt::KeepAspectRatio);
}

void DetWindow::fitViewToActual()
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    _imageView->fitInView(_imageView->rect(), Qt::KeepAspectRatio); // please check this issue.
}
//next Image
void DetWindow::nextImage()
{
    int rowCount=_fileListModel->rowCount(_fileListView->rootIndex());
    int currentRow=_fileListView->currentIndex().row();
    currentRow=qMin(rowCount-1,currentRow+1);
    QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
    _fileListView->rootIndex();
    _fileListView->setCurrentIndex(index);
}
//prev Image
void DetWindow::prevImage()
{
    int currentRow=_fileListView->currentIndex().row();
    currentRow=qMax(0,currentRow-1);
    QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
    _fileListView->rootIndex();
    _fileListView->setCurrentIndex(index);
}
//full Screen
void DetWindow::fullScreen()
{
    auto makeFullscreen = !isFullScreen();

    _fileListView->setVisible(!makeFullscreen);
    setWindowState(makeFullscreen ? Qt::WindowFullScreen : Qt::WindowMaximized);
}

void DetWindow::updateLabelImageSize(QSize imageSize)
{
    _imageSize = imageSize;
    _labelImageInfo->setText(QString(tr("Image: %1 Size: %2 x %3"))
                             .arg(_selectedImageName)
                             .arg(_imageSize.width())
                             .arg(_imageSize.height())
                             .toUtf8());
}

void DetWindow::updateBoxInfo(QRect rect, QString typeName)
{
    if (typeName.isNull() || rect.isNull())
    {
        _labelBoxInfo->setText(QString(tr("Box: x- , y- , w- , h- , type: "))
                               .toUtf8());
        return;
    }
    _boxRect = rect;
    _boxTypeName = typeName;
    _labelBoxInfo->setText(QString(tr("Box: x-%1, y-%2, w-%3, h-%4, type: %5"))
                           .arg(_boxRect.left())
                           .arg(_boxRect.top())
                           .arg(_boxRect.width())
                           .arg(_boxRect.height())
                           .arg(_boxTypeName)
                           .toUtf8());
    _typeNameComboBox->setCurrentText(_boxTypeName);
}

void DetWindow::updateLabelCursorPos(QPointF cursorPos)
{
    _cursorPos = cursorPos;
    _labelCursorPos->setText(QString(tr("Cursor: %1, %2"))
                             .arg((int)_cursorPos.x())
                             .arg((int)_cursorPos.y())
                             .toUtf8());
}
