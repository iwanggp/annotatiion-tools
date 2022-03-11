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

#include "classwindow.h"
#include "mfile.h"

ClassWindow::ClassWindow()
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

void ClassWindow::goHome()
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

void ClassWindow::openFolder()
{
    readListFile();
//    emit isBoxTask();
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
                this, &ClassWindow::onFileSelected);

        QModelIndex index = _fileListModel->index(0, 0, _fileListView->rootIndex());
        _fileListView->setCurrentIndex(index);
        _panAct->setEnabled(true);
        //设置上一张和下一张图片
        int rowCount = _fileListModel->rowCount(_fileListView->rootIndex());
        if (rowCount > 1) {
            _nextAct->setEnabled(true);
            _prevAct->setEnabled(true);
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
void ClassWindow::saveImageNamesToFile(const QString fileName)
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
/**
  *根据索引修改下拉框的值
 * @brief ClassWindow::updateTypeIndex
 * @param index
 */
void ClassWindow::updateTypeIndex(int index)
{
    _typeNameComboBox->setCurrentIndex(index);
}

QStringList ClassWindow::loadTypeNameFromFile(QString filePath)//加载下拉框
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

void ClassWindow::help()
{
    _helpMessageBox.about(this,
                          qApp->translate("ClassWindow", "Help"),
                          qApp->translate("ClassWindow", _helpText));
}

void ClassWindow::about()
{
    sprintf(_aboutText, "<p><b>Image Labeler %d.%d.%d</b> is based on Qt 5.10.1 and FreeImage 3.18.</p>",
            VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
    _aboutMessageBox.about(this,
                           qApp->translate("ClassWindow", "About Image Labeler"),
                           qApp->translate("ClassWindow", _aboutText));
}

/**
 * @brief ClassWindow::createActions add menu actions
 */
void ClassWindow::createActions()
{
    // file menu
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _fileToolBar = addToolBar(tr("File"));
    //go back
    _goHome = new QAction(QIcon(":/images/home.png"), tr("&Go Home..."), this);
    _goHome->setShortcuts(QKeySequence::Back);
    _goHome->setStatusTip(tr("Go Home"));
    connect(_goHome, &QAction::triggered, this, &ClassWindow::goHome);
    _fileMenu->addAction(_goHome);
    _fileToolBar->addAction(_goHome);
    // open folder
    _openAct = new QAction(QIcon(":/images/folder.png"), tr("&Open Folder..."), this);
    _openAct->setShortcuts(QKeySequence::Open);
    _openAct->setStatusTip(tr("Open an image folder"));
    connect(_openAct, &QAction::triggered, this, &ClassWindow::openFolder);
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
    connect(_editAct, &QAction::triggered, this, &ClassWindow::editTypeNameList);
    _editMenu->addAction(_editAct);
    _editToolBar->addAction(_editAct);
    _editAct->setEnabled(false);

    // target type combobox
    _typeNameComboBox = new QComboBox(this);
    _editToolBar->addWidget(_typeNameComboBox);
    _typeNameComboBox->installEventFilter(this);
//    menuBar()->addSeparator();

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
    connect(_panAct, &QAction::toggled, this, &ClassWindow::panImage);
    _viewMenu->addAction(_panAct);
    _viewToolBar->addAction(_panAct);

    // fit to window
    _fitToWindowAct = new QAction(QIcon(":/images/fit-to-window.png"), tr("&Fit To Window"), this);
    _fitToWindowAct->setShortcut(tr("Ctrl+F"));
    _fitToWindowAct->setStatusTip(tr("Fit View To Window"));
    connect(_fitToWindowAct, &QAction::triggered, this, &ClassWindow::fitViewToWindow);
    _fitToWindowAct->setCheckable(true);
    _fitToWindowAct->setChecked(true);
    _viewMenu->addAction(_fitToWindowAct);
    _viewToolBar->addAction(_fitToWindowAct);

    // actual size
    _actualSizeAct = new QAction(QIcon(":/images/actual-size.png"), tr("&Actual Size"), this);
    _actualSizeAct->setShortcut(tr("Ctrl+R"));
    _actualSizeAct->setStatusTip(tr("Fit View To Actual Size"));
    connect(_actualSizeAct, &QAction::triggered, this, &ClassWindow::fitViewToActual);
    _viewMenu->addAction(_actualSizeAct);
    _viewToolBar->addAction(_actualSizeAct);

    _viewMenu->addSeparator();

    // full screen
    _fullscreenAct = new QAction(QIcon(":/images/fullscreen.png"), tr("&Full Screen"), this);
    _fullscreenAct->setShortcut(tr("Alt+Enter"));
    _fullscreenAct->setStatusTip(tr("Full Screen"));
    connect(_fullscreenAct, &QAction::triggered, this, &ClassWindow::fullScreen);
    _viewMenu->addAction(_fullscreenAct);
    _viewToolBar->addAction(_fullscreenAct);

    //Prev Image
    // Prev Image
    _prevAct = new QAction(QIcon(":/images/prev.png"), tr("&Prev"), this);
    _prevAct->setStatusTip(tr("Prev"));
    _prevAct->setShortcut(tr("A"));
    _prevAct->setEnabled(false);
    connect(_prevAct, &QAction::triggered, this, &ClassWindow::prevImage);
    _viewMenu->addAction(_prevAct);
    _viewToolBar->addAction(_prevAct);
    // Next Image
    _nextAct = new QAction(QIcon(":/images/next.png"), tr("&Next"), this);
    _nextAct->setStatusTip(tr("Next"));
    _nextAct->setShortcut(tr("D"));
    _nextAct->setEnabled(false);
    connect(_nextAct, &QAction::triggered, this, &ClassWindow::nextImage);
    _viewMenu->addAction(_nextAct);
    _viewToolBar->addAction(_nextAct);

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
    connect(_zhCNAct, &QAction::triggered, this, &ClassWindow::switchLanguage);

    // English
    _enUSAct = _languageMenu->addAction(QIcon(":/images/en_US.png"),
                                        tr("&English"));
    _languageMenu->addAction(_enUSAct);
    //    _helpToolBar->addAction(_enUSAct);
    connect(_enUSAct, &QAction::triggered, this, &ClassWindow::switchLanguage);

    // help
    _helpAct = _helpMenu->addAction(QIcon(":/images/help.png"),
                                    tr("&Help"), this, &ClassWindow::help);
    _helpAct->setStatusTip(tr("Help"));
    _helpAct->setShortcut(QKeySequence::HelpContents);
    //    _helpToolBar->addAction(_helpAct);

    // about
    _aboutAct = _helpMenu->addAction(QIcon(":/images/about.png"),
                                     tr("&About"), this, &ClassWindow::about);
    _aboutAct->setStatusTip(tr("About Image Labeler"));
    //    _helpToolBar->addAction(_aboutAct);
}

void ClassWindow::switchLanguage()
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

void ClassWindow::retranslate()
{
    // file menu
    _fileMenu->setTitle(tr("&File"));
    //    fileToolBar->setTitle(tr("File"));

    // open folder
    _openAct->setText(tr("&Open Folder..."));
    _openAct->setStatusTip(tr("Open an image folder"));


    // edit menu
    _editMenu->setTitle(tr("&Edit"));

    // edit target type
    _editAct ->setText(tr("T&arget Type"));
    _editAct->setShortcut(tr("Ctrl+T"));
    _editAct->setStatusTip(tr("Edit Target Type"));


    // view menu
    _viewMenu->setTitle(tr("&View"));
    //    viewToolBar->setTitle(tr("View"));

    // pan
    _panAct->setText(tr("&Pan"));
    _panAct->setShortcut(tr("Ctrl+P"));
    _panAct->setStatusTip(tr("Pan Image"));
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
}

void ClassWindow::createCentralWindow()
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

    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_editImageIndex, 1);
    _editImageIndex->setHidden(true);
    this->statusBar()->addPermanentWidget(_labelImageIndex, 1);
    this->statusBar()->addPermanentWidget(new QLabel(), 1);
    this->statusBar()->addPermanentWidget(_labelImageInfo, 1);
}

void ClassWindow::closeEvent(QCloseEvent *event)
{
    if (_imageScene) {
        delete _imageScene;
    }
}

void ClassWindow::updateActions()
{
    _zoomInAct->setEnabled(!_fitToWindowAct->isChecked());
    _zoomOutAct->setEnabled(!_fitToWindowAct->isChecked());
}

void ClassWindow::selectFile()
{
    int row = _editImageIndex->text().toInt();
    QModelIndex index = _fileListModel->index(row-1, 0, _fileListView->rootIndex());
    _fileListView->setCurrentIndex(index);
}

void ClassWindow::onFileSelected(const QItemSelection& selected, const QItemSelection& deselected)
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

void ClassWindow::displayImageView(QString imageFilePath)
{
    if (_imageScene) {
        delete _imageScene;
    }
    _imageScene = new CustomScene(this);

    _imageScene->setTypeNameList(_typeNameList);
    _imageScene->setTypeName(_typeNameComboBox->currentText());
    _imageScene->setClassTask(true);
    _imageScene->installEventFilter(this);
    connect(_imageScene, SIGNAL(imageLoaded(QSize)), this, SLOT(updateLabelImageSize(QSize)));
    connect(_typeNameComboBox, SIGNAL(activated(QString)), _imageScene, SLOT(changeBoxTypeName(QString)));
    connect(_imageScene, SIGNAL(typeChange(int)), this, SLOT(updateTypeIndex(int)));//改变下拉框的值
    _imageScene->loadImage(imageFilePath);
    _isImageLoaded = true;
    _imageView->setScene(_imageScene);

    _panAct->setChecked(false);
    _fitToWindowAct->setChecked(true);
    fitViewToWindow();

}

void ClassWindow::showEvent(QShowEvent* event)
{
    fitViewToWindow();
}

void ClassWindow::resizeEvent(QResizeEvent* event)
{
    fitViewToWindow();
}

bool ClassWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _imageScene || obj == _typeNameComboBox || obj == this) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            // use Key_Down and Key_Up received by _viewScene and ClassWindow to select image in fileListView
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


void ClassWindow::panImage(bool checked)
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    _imageView->panImage(checked);
    _imageScene->panImage(checked);
}

void ClassWindow::editTypeNameList()
{
    readListFile();
}



//add by asf
void ClassWindow::readListFile()
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
        int currentRow=_fileListView->currentIndex().row();
        if(currentRow>=0){
            QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
            displayImageView(_fileListModel->filePath(index));
        }

    }else{
        return;
    }
}

void ClassWindow::zoomIn()
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    _imageView->scale(1.2, 1.2);
}

void ClassWindow::zoomOut()
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    _imageView->scale(0.8, 0.8);
}

void ClassWindow::wheelEvent(QWheelEvent *event)//鼠标滚轮事件
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    qreal newZoom = 1 + (event->delta() / 120.0) * 0.05;
    _imageView->scale(newZoom, newZoom);
}

void ClassWindow::fitViewToWindow()
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

void ClassWindow::fitViewToActual()
{
    if (!_isImageLoaded)
        return;

    if (_fitToWindowAct->isChecked())
        _fitToWindowAct->setChecked(false);

    _imageView->fitInView(_imageView->rect(), Qt::KeepAspectRatio); // please check this issue.
}
//next Image
void ClassWindow::nextImage()
{
    int rowCount=_fileListModel->rowCount(_fileListView->rootIndex());
    int currentRow=_fileListView->currentIndex().row();
    currentRow=qMin(rowCount-1,currentRow+1);
    QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
    _fileListView->rootIndex();
    _fileListView->setCurrentIndex(index);
}
//prev Image
void ClassWindow::prevImage()
{
    int currentRow=_fileListView->currentIndex().row();
    currentRow=qMax(0,currentRow-1);
    QModelIndex index = _fileListModel->index(currentRow,  0, _fileListView->rootIndex());
    _fileListView->rootIndex();
    _fileListView->setCurrentIndex(index);
}
//full Screen
void ClassWindow::fullScreen()
{
    auto makeFullscreen = !isFullScreen();

    _fileListView->setVisible(!makeFullscreen);
    setWindowState(makeFullscreen ? Qt::WindowFullScreen : Qt::WindowMaximized);
}

void ClassWindow::updateLabelImageSize(QSize imageSize)
{
    _imageSize = imageSize;
    _labelImageInfo->setText(QString(tr("Image: %1 Size: %2 x %3"))
                             .arg(_selectedImageName)
                             .arg(_imageSize.width())
                             .arg(_imageSize.height())
                             .toUtf8());
}

