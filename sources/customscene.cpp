#include "customscene.h"
#include <QtDebug>
#include <QScrollBar>
//添加Json模块
#include<QJsonDocument>
#include<QByteArray>
#include<QVariant>
#include<QMap>
#include<QList>
#include<QString>
#include<QFileDialog>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonParseError>
#include<QJsonValue>
#include<QProgressDialog>
#include<opencv2/opencv.hpp>
#include<QFile>
#include<QDir>
#include<QFileInfo>
#include<QDateTime>
#include<QUuid>
#include<QTextCodec>
using namespace cv;
CustomScene::CustomScene(QObject* parent):
    QGraphicsScene(parent),
    _image(nullptr),
    _pixmapItem(nullptr),
    _boxItem(nullptr),
    _isDrawing(false),
    _undoStack(new QUndoStack),
    _clickedPos(QPointF(0,0))
{

}

void CustomScene::clearAll()
{
    qDebug()<<"clearAll is run............."<<_isBoxTask;
    if(_isBoxTask) saveBoxItemsToFile();//保存检测文件
    else if(_isSegTask) savePointItemsToFile();
    else if(_isClassTask) saveClassToFile();
    else saveOCRPointItemsToFile();

    if (_image != nullptr) {
        delete _image;
        _image = nullptr;
    }
    if (_pixmapItem != nullptr) {
        delete _pixmapItem;
        _pixmapItem = nullptr;
    }
    if (_boxItemMimeData) {
        delete _boxItemMimeData;
    }//此处有个bug

    foreach (QGraphicsItem *item, this->items()) {
        if ((item->type() == QGraphicsItem::UserType+1) ||
                (item->type() == QGraphicsPixmapItem::Type) ) {
            this->removeItem(item);
            delete item;
        }
    }

    if (_undoStack != nullptr) {
        _undoStack->clear();
        delete _undoStack;
        _undoStack = nullptr;
    }
    if(_isOCRTask) emit labelChange("");
    this->items().clear();
    this->clear();
}

void CustomScene::loadImage(QString filename)
{
    // Get image format
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename.toLocal8Bit(), 0);
    if(fif == FIF_UNKNOWN)
        fif = FreeImage_GetFIFFromFilename(filename.toLocal8Bit());
    if(fif == FIF_UNKNOWN)
        return;

    // Load image if possible
    FIBITMAP *dib = nullptr;
    if(FreeImage_FIFSupportsReading(fif)) {
        dib = FreeImage_Load(fif, filename.toLocal8Bit());
        if(dib == nullptr)
            return;
    } else
        return;

    // Convert to 24bits and save to memory as JPEG
    FIMEMORY *stream = FreeImage_OpenMemory();
    // FreeImage can only save 24-bit highcolor or 8-bit greyscale/palette bitmaps as JPEG
    FIBITMAP *dib1 = FreeImage_ConvertTo24Bits(dib);
    FreeImage_SaveToMemory(FIF_JPEG, dib1, stream);

    // Load JPEG data
    BYTE *mem_buffer = nullptr;
    DWORD size_in_bytes = 0;
    FreeImage_AcquireMemory(stream, &mem_buffer, &size_in_bytes);

    // Load raw data into QImage and return
    QByteArray array = QByteArray::fromRawData((char*)mem_buffer, (int)size_in_bytes);

    _image = new QImage();
    _image->loadFromData(array);
    //_image = new QImage(filename);
    _pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(*_image));
    _pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    this->addItem(_pixmapItem);

    emit imageLoaded(_image->size());
    setSceneRect(_image->rect());

    // load box items
    QFileInfo info(filename);
    _fileName=filename;
    _currentPath=info.path();
    _boxItemFileName = info.path() + "/" + info.completeBaseName() + ".txt";
    _classFileName = info.path() + "/" + info.completeBaseName()+"_class" + ".txt";
    _boxItemVOCFileName = info.path() + "/" + info.completeBaseName() + "_voc.txt";
    _trainVOCFileName = info.path() + "/train.txt";
    _pointItemFileName = info.path() + "/" + info.completeBaseName() + ".json";
    if(_isBoxTask) loadBoxItemsFromFile();// 加载标签
    else if(_isClassTask){
        loadClassFromFile();
    }
    else loadPointItemsFromFile();
    array.clear();
    FreeImage_CloseMemory(stream);
    FreeImage_Unload(dib);
    FreeImage_Unload(dib1);
}
/**
 * @brief CustomScene::loadBoxItemsFromFile
 * 加载检测标注结果
 */
void CustomScene::loadBoxItemsFromFile()
{
    QFile file(_boxItemFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QPoint zero(0, 0);
    QRect fatherRect(zero, _image->size());
    qreal x,y,w,h;
    int index;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList info = in.readLine().split(" ");
        if(info.size() >= 5) {
            index = info.at(0).toInt();
            if(index==-1 || index>_typeNameList.count()){
                QMessageBox::critical(0 , "Alert" , "标签文件内容有误", QMessageBox::Ok | QMessageBox::Default , QMessageBox::Cancel | QMessageBox::Escape , 	0 );
                return;
            }
            w = info.at(3).toFloat() * _image->width();
            h = info.at(4).toFloat() * _image->height();
            x = info.at(1).toFloat() * _image->width() - w/2;
            y = info.at(2).toFloat() * _image->height() - h/2;

            BoxItem *b = new BoxItem(fatherRect, _image->size(), _typeNameList, _typeNameList.at(index));
            b->setRect(x,y,w,h);
            this->registerItem(b);
            if (!b->rect().isNull())
                this->addItem(b);
        }
    }
    file.close();
}
/**
  * 加载分类文件
 * @brief CustomScene::loadClassFromFile
 */
void CustomScene::loadClassFromFile()
{
    QFile file(_classFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList info = in.readLine().split(" ");
        int index=info.at(0).toInt();
        qDebug()<<"the class is........."<<index;
        QString label=_typeNameList.at(index);
        qDebug()<<"the class is........."<<index<<"---"<<label;
        emit typeChange(index);
    }
    file.close();
}
/**
 * @brief CustomScene::loadPointItemsFromFile
 * 加载已经标注好的json文件
 */
void CustomScene::loadPointItemsFromFile()
{
    QFile loadFile(_pointItemFileName);

    if(!loadFile.open(QIODevice::ReadOnly))
    {

        qDebug()<<"could open projects json";
    }
    QByteArray allData=loadFile.readAll();
    loadFile.close();

    QPoint zero(0, 0);
    QRect fatherRect(zero, _image->size());
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData,&json_error));
    if(json_error.error!=QJsonParseError::NoError)
    {
        qDebug()<<"json error";
        //        return false;
    }
    QJsonObject rootObj=jsonDoc.object();
    if(rootObj.contains("shapes"))
    {
        QJsonArray ss=rootObj.value("shapes").toArray();
        for(int i=0;i<ss.size();i++)
        {
            QJsonObject subObj=ss[i].toObject();
            QJsonArray temp=subObj.value("points").toArray();
            if(temp.isEmpty()) return;
            QString label=subObj.value("label").toString();
            if(_isOCRTask) emit labelChange(label);
            qDebug()<<"the OCR label is........"<<label;
            QList<QPointF> pointsList;
            for(int i=0;i<temp.size();i++)
            {
                pointsList.push_back(QPointF(temp[i].toArray()[0].toDouble(),round(temp[i].toArray()[1].toDouble())));

            }
            pointsList.push_back(QPointF(temp[0].toArray()[0].toDouble(),round(temp[0].toArray()[1].toDouble())));//封口
            PointItem *p = new PointItem(fatherRect, _image->size(), _typeNameList, label);
            p->setPointList(pointsList);
            this->registerItem(p);
            if(!p->points().isEmpty()) this->addItem(p);
        }

    }
}
void CustomScene::registerItem(BoxItem *b)
{
    connect(b, SIGNAL(typeNameChanged(QString)), this, SLOT(changeBoxTypeName(QString)));
    connect(b, SIGNAL(boxSelected(QRect, QString)), this, SIGNAL(boxSelected(QRect, QString)));
    connect(b, SIGNAL(boxSelected(QRect, QString)), this, SLOT(selectedBoxItemInfo(QRect,QString)));
    connect(b, SIGNAL(stretchCompleted(QRectF, QRectF)), this, SLOT(moveBox(QRectF, QRectF)));
    connect(b, SIGNAL(moveCompleted(QRectF, QRectF)), this, SLOT(moveBox(QRectF, QRectF)));
    b->installEventFilter(this);
}
//定义画点的信号与槽函数
void CustomScene::registerItem(PointItem *p)
{
    connect(p,SIGNAL(closePoint()),this,SLOT(closePoint()));
    connect(p,SIGNAL(getCurentPointsList()),this,SLOT(setCurentPointsList()));
    connect(p,SIGNAL(typeNameChanged(QString)), this, SLOT(changePointTypeName(QString)));
    connect(p,SIGNAL(getLabel(QString)), this, SLOT(changePointLabelName(QString)));
    connect(p,SIGNAL(cleanUndoStack()), this, SLOT(clearUndoStack()));
    connect(p, SIGNAL(moveCompleted(QList<QPointF>, QList<QPointF>)), this, SLOT(movePoints(QList<QPointF>, QList<QPointF>)));
    p->installEventFilter(this);
}
/**
  * 保存检测标注的文件
 * @brief CustomScene::saveBoxItemsToFile
 */
void CustomScene::saveBoxItemsToFile()
{
    QFile file(_boxItemFileName);
//    QFile file_voc(_boxItemVOCFileName);//VOC格式的标签文件
    file.open(QIODevice::WriteOnly | QIODevice::Text);
//    file_voc.open(QIODevice::WriteOnly | QIODevice::Text);
    qreal label[4];
    int voc_label[4];
    QTextStream out(&file);
//    QTextStream out_voc(&file_voc);
//    QString voc_s;
//    voc_s.sprintf("%s ",_fileName.toStdString().c_str());
//    out_voc<<voc_s;
    foreach (QGraphicsItem *item, this->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            BoxItem *b = qgraphicsitem_cast<BoxItem *>(item);
            b->rect(label);
//            b->rect_voc(voc_label);
            QString s;
            s.sprintf("%d %f %f %f %f\n",
                      _typeNameList.indexOf(b->typeName()),
                      label[0],label[1],label[2],label[3]);
//            voc_s.sprintf("%d,%d,%d,%d,%d ",
//                          voc_label[0],voc_label[1],voc_label[2],voc_label[3],_typeNameList.indexOf(b->typeName()));
            out << s;
//            out_voc<<voc_s;
        }
    }
    file.close();
//    file_voc.close();
}
/**
  *保存分类任务的结果
 * @brief CustomScene::saveClassToFile
 */
void CustomScene::saveClassToFile()
{
    QFile file(_classFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    QString s;
    s.sprintf("%d",_typeNameList.indexOf(_typeName));
    out<<s;
    file.close();
}

/**
 * @brief CustomScene::savePointItemsToFile
 * 将分割结果保存为json文件
 */
void CustomScene::savePointItemsToFile()
{
    QFile file_yolo(_boxItemFileName);
    file_yolo.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_yolo(&file_yolo);
    QFile file(_pointItemFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    QList<QPointF> _pointsList;
    QMap<int,QList<QPoint>> _yoloItem;
    QJsonDocument jsonDoc;
    QJsonObject jsonObject,shapeObject;
    QJsonArray shapeArray;
    jsonObject.insert("version","1.0.0");
    jsonObject.insert("flags","");
    jsonObject.insert("imagePath",_fileName);
    jsonObject.insert("imageData","");
    jsonObject.insert("imageHeight",_image->height());
    jsonObject.insert("imageWidth",_image->width());
    foreach (QGraphicsItem *item, this->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            PointItem *p = qgraphicsitem_cast<PointItem *>(item);
            _pointsList=p->points();
            if(_pointsList.isEmpty()) continue;//防止为空时候出错
            shapeObject=writeQJson(p->typeName(),_pointsList);
            shapeArray.append(shapeObject);//标签的数目
            qreal *_result=write2YOLO(_image,_pointsList);
            QString s;
            s.sprintf("%d %f %f %f %f\n",
                      _typeNameList.indexOf(p->typeName()),
                      _result[0],_result[1],_result[2],_result[3]);
            out_yolo << s;
        }
    }
    jsonObject.insert("shapes",QJsonValue(shapeArray));
    jsonDoc.setObject(jsonObject);
    file.write(jsonDoc.toJson());
    file.close();
    file_yolo.close();
}
/**
 * @brief CustomScene::saveOCRPointItemsToFile
 */
void CustomScene::saveOCRPointItemsToFile()
{
    QFile file_yolo(_boxItemFileName);
    file_yolo.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_yolo(&file_yolo);
    out_yolo.setCodec(QTextCodec::codecForName("utf-8"));
    QFile file(_pointItemFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    QList<QPointF> _pointsList;
    QMap<int,QList<QPoint>> _yoloItem;
    QJsonDocument jsonDoc;
    QJsonObject jsonObject,shapeObject;
    QJsonArray shapeArray;
    jsonObject.insert("version","1.0.0");
    jsonObject.insert("flags","");
    jsonObject.insert("imagePath",_fileName);
    jsonObject.insert("imageData","");
    jsonObject.insert("imageHeight",_image->height());
    jsonObject.insert("imageWidth",_image->width());
    foreach (QGraphicsItem *item, this->items()) {
        QString s;
        if (item->type() == QGraphicsItem::UserType+1) {
            PointItem *p = qgraphicsitem_cast<PointItem *>(item);
            _pointsList=p->points();
            if(_pointsList.isEmpty()) continue;//防止为空时候出错
            shapeObject=writeQJson(p->typeName(),_pointsList);
            shapeArray.append(shapeObject);//标签的数目
            //获得坐标点的值
            QJsonArray pointArray=shapeObject["points"].toArray();
            for(int i=0;i<pointArray.size();i++){
                double x=pointArray[i].toArray()[0].toDouble();
                double y=pointArray[i].toArray()[1].toDouble();
                s.sprintf("%f %f ",x,y);
                out_yolo << s;
            }
            s.sprintf("%.50s\n",p->typeName().toStdString().c_str());//将QString转换为标准的string进行保存
            out_yolo << s;
        }
    }
    jsonObject.insert("shapes",QJsonValue(shapeArray));
    jsonDoc.setObject(jsonObject);
    file.write(jsonDoc.toJson());
    file.close();
    file_yolo.close();
}
/**
 * @brief CustomScene::writeQJson
 * @param _mapList
 * 将点的信息写到json文件中
 * {
    "flags": "",
    "imageData": "",
    "imageHeight": "",
    "imagePath": "",
    "imageWidth": "",
    "shapes": [
        {
            "flags": "",
            "group_id": "null",
            "label": "vvvv",
            "points": [
                [
                    117.42,
                    617.42
                ]
            ],
            "shape_type": "polygon"
        },
    ],
    "version": "1.0.0"
}

 */
QJsonObject CustomScene::writeQJson(QString label,QList<QPointF> _pointsList){
    QJsonObject shapeObject;//定义整体object和shapeObject
    QJsonArray pointsArray,ponitArray;//定义shapes和points
    for(int i=0;i<_pointsList.size()-1;i++){
        ponitArray.append(_pointsList.at(i).x());//各个点坐标
        ponitArray.append(_pointsList.at(i).y());//各个点坐标
        pointsArray.append(ponitArray);
        ponitArray.removeFirst();
        ponitArray.removeLast();
    }
    shapeObject.insert("label",label);//定义标签
    shapeObject.insert("points",QJsonValue(pointsArray));
    shapeObject.insert("group_id","null");
    shapeObject.insert("shape_type","polygon");//标注的形状
    shapeObject.insert("flags","");
    return shapeObject;
}
/**
 * @brief CustomScene::write2YOLO
 * @param _image 图像的信息
 * @param _pointsList 坐标点
 */
qreal* CustomScene::write2YOLO(QImage *_image,QList<QPointF> _pointsList)
{
    QVector<double> _pointX,_pointY;//定义两个坐标的vector
    QList<QPoint> _yoloItem;
    for(auto point:_pointsList){
        _pointX.push_back(point.x());
        _pointY.push_back(point.y());
    }
    //(_xMin,_yMin)左上和右下(_yMin,_yMax)
    auto _xMin=std::min_element(std::begin(_pointX),std::end(_pointX));
    auto _xMax=std::max_element(std::begin(_pointX),std::end(_pointX));
    auto _yMin=std::min_element(std::begin(_pointY),std::end(_pointY));
    auto _yMax=std::max_element(std::begin(_pointY),std::end(_pointY));
    qreal dw=1.0/_image->width();
    qreal dh=1.0/_image->height();
    qreal x=(*_xMin+*_xMax)/2.0-1;
    qreal y=(*_yMin+*_yMax)/2.0-1;
    int w=*_xMax-*_xMin;
    int h=*_yMax-*_yMin;
    qreal _x=x*dw;
    qreal _w=w*dw;
    qreal _y=y*dh;
    qreal _h=h*dh;
    qreal _result[]={_x,_y,_w,_h};
    qreal* temp = new qreal[4];
    for ( int i =0; i < 4; i++) temp[i] = (qreal)_result[i];//C++要求必须这样写
    return temp;
}
/**
 * @brief CustomScene::deleteBoxItems
 */
void CustomScene::deleteBoxItems()
{
    QList<QGraphicsItem *> boxList;
    foreach (QGraphicsItem *item, this->selectedItems()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            boxList.append(item);
        }
    }

    //undo处理
    if (boxList.count() > 0) {
        _undoStack->push(new RemoveBoxesCommand(this, boxList));
        QApplication::setOverrideCursor(qgraphicsitem_cast<BoxItem *>(boxList.at(0))->oldCursor());
    }
    boxList.clear();

}
/**
 * @brief CustomScene::saveBoxItemsToVOC
 * 将文件导出VOC训练数据，生成训练数据
 */
void CustomScene::saveBoxItemsToVOC()
{
    QDir dir(_currentPath);

    if(!dir.exists()){
        return;
    }
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::DirsFirst);//文件夹排在前面
    QStringList filterst;//文件过滤器
    filterst << QString("*.jpg") << QString("*.jpeg")<< QString("*.png")<< QString("*.bmp");
    QFileInfoList list=dir.entryInfoList(filterst);//获取本文件内所有文件信息
    QFile file(_trainVOCFileName);
    if (file.exists()) file.remove();//有文件就删除
    file.open(QIODevice::WriteOnly | QIODevice::Text|QIODevice::Append);
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("utf-8"));
    QString s;
    int fileCount=list.count();
    if(fileCount<1){
        return;
    }
    QProgressDialog dialog(tr("Generating  train sets please waiting......."), "", 0, fileCount, nullptr);//弹出生成语义图过程的进度框
    dialog.show();
    for(int i=0;i<fileCount;i++)
    {
        QFileInfo fileInfo=list[i];//获取每个文件信息
//        QString fileName=fileInfo.baseName();
        QString fileName=fileInfo.absoluteFilePath();
//        if (fileName.contains("voc"))
//        {
        dialog.setValue(i);
        QCoreApplication::processEvents();
        if(dialog.wasCanceled())
            break;
//        QFile file(fileInfo.absoluteFilePath());
//        file.open(QIODevice::ReadOnly | QIODevice::Text);
//        QTextStream in(&file);
//        while (!in.atEnd()) {
//            QString info = in.readLine();
//            if(info.endsWith(' ')) info.chop(1);//删除末尾的空格
//            qDebug()<<"VOC.........the info ........."<<info;
//            out << s;
//            s.sprintf("%s\n",info.toStdString().c_str());
//        }
//        }
        qDebug()<<"VOC.........the filename ........."<<fileName;
        out << s;
        s.sprintf("%s\n",fileName.toStdString().c_str());
    }
    file.close();
}
/**
 * @brief CustomScene::augMent
 * 图像增强功能
 */
void CustomScene::augMent()
{
    augDialog.setModal(true);
    augDialog.show();
    connect(&augDialog,SIGNAL(augConfirm(int,int,bool,bool,bool,QString)),this,SLOT(augMentConfirm(int,int,bool,bool,bool,QString)));//这里接受到信号
    //    augMentConfirm(5,30,true,true,true,false);//这里开始传值
}
/** 确认图像增强
 * @brief CustomScene::augMentConfirm
 * @param num_value 扩充的总数
 * @param degree 旋转的角度
 * @param is_verti 是否垂直翻转
 * @param is_rote 是否水平翻转
 * @param is_mir 是否做镜面转换
 * @param is_gv 是否更改gv值
 */
void CustomScene::augMentConfirm(int num_value,int degree,bool is_verti,bool is_rote,bool is_mir,QString gv_dir)
{
    qDebug()<<"the num is ......."<<num_value<<"the ang is ......"<<degree<<"is is_verti......"<<is_verti<<"is_rote ......."<<is_rote<<"is mir ......."<<is_mir<<"is_gv ......."<<gv_dir;
    QDir dir(_currentPath);
    if(!dir.exists()){
        return;
    }
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::DirsFirst);//文件夹排在前面
    QFileInfoList list=dir.entryInfoList();//获取本文件内所有文件信息
    int fileCount=list.count();
    if(fileCount<1){
        return;
    }
    QProgressDialog dialog(tr("Augmenting  Pics please waiting......."), "", 0, fileCount, nullptr);//弹出生成语义图过程的进度框
    dialog.show();
    cv::Mat augPic;
    QDateTime current_data_time=QDateTime::currentDateTime();//获得当前日期函数
    QString strStartTime=current_data_time.toString("yyyyddMMhhmmsszzz");//获得当前日期
    for(int i=0;i<fileCount;i++)
    {
        QFileInfo fileInfo=list[i];//获取每个文件信息
        bool isAug=true;
        if (fileInfo.suffix()=="png"||fileInfo.suffix()=="jpg"||fileInfo.suffix()=="jpeg"||fileInfo.suffix()=="bmp"
                ||fileInfo.suffix()=="gif"){
            dialog.setValue(i);
            QCoreApplication::processEvents();
            if(dialog.wasCanceled())
                break;
            QString _fileName=fileInfo.fileName();
            QString fileName=_fileName.left(_fileName.lastIndexOf('.'));

            qDebug()<<"the fileName......"<<fileName.left(fileName.lastIndexOf('.'));;
            QString filePath=fileInfo.absoluteFilePath();
            cv::Mat img=cv::imread(filePath.toLocal8Bit().toStdString());
            int width=img.cols;//图片的宽度
            int height=img.rows;//图片的高度
            cv::Point point_L_U;//左上点坐标
            cv::Point point_R_L;//右下点坐标
            int a=img.cols/2;//围绕中心点进行旋转
            int b=img.rows/2;//围绕中心点进行旋转
            QString yoloAnnoFile=_currentPath+"/"+fileName+".txt";
            qDebug()<<"the yoloannofile is......"<<yoloAnnoFile;
            QFile file22(yoloAnnoFile);
            file22.open(QIODevice::ReadOnly | QIODevice::Text);
            qreal x,y,w,h;
            int id;//标签编号
            int index;
            int *voc;
            QList<int*> voc_lst;
            QTextStream in(&file22);
            while (!in.atEnd()) {//读取标注文件
                QStringList info = in.readLine().split(" ");
                qDebug()<<"the info is......"<<info;
                if(info.size() >= 5) {
                    index = info.at(0).toInt();
                    if(index==-1 || index>_typeNameList.count()){
                        QMessageBox::critical(0 , "Alert" , "标签文件内容有误", QMessageBox::Ok | QMessageBox::Default , QMessageBox::Cancel | QMessageBox::Escape , 	0 );
                        return;
                    }
                    id=info.at(0).toInt();
                    w = info.at(3).toFloat() * width;
                    h = info.at(4).toFloat() * height;
                    x = info.at(1).toFloat() * width;
                    y = info.at(2).toFloat() * height;
                    voc=YOLO2VOC(id,w,h,x,y);
                    voc_lst.push_back(voc);//将标注结果放到list中
                    qDebug()<<"the voc_lst is......"<<voc_lst;
                }
            }
            file22.close();
            if(degree>0&&num_value>0)
            {
                for (int i=0;i<num_value;i++) {
                    isAug=true;
                    int rote_degree=(rand()%(degree)+1);//随机选择一个旋转角度
                    QString augPicBasePath=_currentPath+"/"+fileName+"_"+strStartTime+"_"+QString::number(rote_degree);
                    generateImgCoor(augPicBasePath,rote_degree,isAug,img,voc_lst);//生成旋转图片和相应的坐标
                }
            }
            if(is_verti){//垂直翻转
                int rote_degree=90;
                QString augPicBasePath=_currentPath+"/"+fileName+"_"+strStartTime+"_"+QString::number(rote_degree);
                generateImgCoor(augPicBasePath,rote_degree,isAug,img,voc_lst);//生成旋转图片和相应的坐标
            }
            if(is_rote){//垂直翻转
                int rote_degree=180;
                QString augPicBasePath=_currentPath+"/"+fileName+"_"+strStartTime+"_"+QString::number(rote_degree);
                generateImgCoor(augPicBasePath,rote_degree,isAug,img,voc_lst);//生成旋转图片和相应的坐标
            }
            if(is_mir){//垂直翻转
                int rote_degree=270;
                QString augPicBasePath=_currentPath+"/"+fileName+"_"+strStartTime+"_"+QString::number(rote_degree);
                generateImgCoor(augPicBasePath,rote_degree,isAug,img,voc_lst);//生成旋转图片和相应的坐标
            }

        }
    }
    if(!gv_dir.isEmpty()){//增加gv值改变的逻辑代码
        gvAugMent(gv_dir);

    }
}
/**
 * @brief CustomScene::exportMaskPic
 * 将标注结果导出为语义图
 */
void CustomScene::exportMaskPic()
{
    QString maskImageDir = QFileDialog::getExistingDirectory(nullptr, tr("Open Directory"),
                                                             "",QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(maskImageDir.isNull()){
        QMessageBox::information(NULL, tr("Warning!!!"), tr("Dir is Empty"));
        return;
    }else if(maskImageDir==_currentPath)
    {
        QMessageBox::information(NULL, tr("Warning!!!"), tr("Can't Choose the Same Dir"));
        return;
    }
    QDir mask_dir(maskImageDir);
    if(!mask_dir.exists()){
        return;
    }
    QDir dir(_currentPath);

    if(!dir.exists()){
        return;
    }
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::DirsFirst);//文件夹排在前面
    QFileInfoList list=dir.entryInfoList();//获取本文件内所有文件信息


    int fileCount=list.count();
    if(fileCount<1){
        qDebug()<<"can't export........";
    }
    QProgressDialog dialog(tr("Generating Mask Pics please waiting......."), "", 0, fileCount, nullptr);//弹出生成语义图过程的进度框
    dialog.show();
    for(int i=0;i<fileCount;i++)
    {
        QFileInfo fileInfo=list[i];//获取每个文件信息
        QString suffix=fileInfo.suffix();//获取后缀名

        if(QString::compare(suffix,QString("json"),Qt::CaseSensitive)==0)
        {
            dialog.setValue(i);
            QCoreApplication::processEvents();
            if(dialog.wasCanceled())
                break;
            QString filePath=fileInfo.absoluteFilePath();//获取文件的绝对路径
            QJsonObject rootObj=parasJson(filePath);
            if(rootObj.contains("imagePath")){
                QString _fileFull,_fileName;//文件的路径名和文件名
                _fileFull=rootObj.value("imagePath").toString();
                QFileInfo fileInfo=QFileInfo(_fileFull);
                _fileName=fileInfo.fileName();
                qDebug()<<"parse Json ........."<<_fileName;
                try {
                    Mat _mask=json2MaskPic(filePath);
                    cv::imwrite((mask_dir.absoluteFilePath(_fileName).toStdString()),_mask);
                } catch (cv::Exception&e) {
                    const char* err_msg = e.what();
                    std::cout << "exception caught: " << err_msg << std::endl;
                }
            }
        }

    }

}
/**
 * @brief CustomScene::json2MaskPic
 * @param filePath
 * @return
 * 获取图片的语义信息
 */
cv::Mat CustomScene::json2MaskPic(QString filePath)
{
    Mat mask;
    QJsonObject rootObj=parasJson(filePath);
    if(rootObj.contains("imageHeight")&&rootObj.contains("imageWidth"))
    {
        int height=rootObj.value("imageHeight").toInt();
        int width=rootObj.value("imageWidth").toInt();
        mask=Mat::zeros(height,width,CV_8UC1);//全黑的单通道图片
    }else{
        qDebug()<<"json error";
    }
    if(rootObj.contains("shapes"))
    {
        QJsonArray ss=rootObj.value("shapes").toArray();
        std::vector<std::vector<cv::Point>> Pts;
        for(int i=0;i<ss.size();i++)
        {
            QJsonObject subObj=ss[i].toObject();
            QJsonArray temp=subObj.value("points").toArray();
            QString label=subObj.value("label").toString();
            qDebug()<<"the label is......"<<label<<"the label index is ....."<<_typeNameList.indexOf(label);//获取当前标签值

            std::vector<cv::Point> tPts;
            for(int i=0;i<temp.size();i++)
            {
                qDebug()<<"single point is....."<<QString::number(temp[i].toArray()[0].toDouble(),'f',10);
                tPts.push_back(cv::Point(round(temp[i].toArray()[0].toDouble()),round(temp[i].toArray()[1].toDouble())));
            }
            qDebug()<<"typeName is ......"<<_typeNameList.indexOf(label);
            cv::fillPoly(mask,tPts,cv::Scalar((_typeNameList.indexOf(label)+1)*255));//填充多边形,并以类别索引做为填充颜色
            //            cv::fillPoly(mask,tPts,cv::Scalar((_typeNameList.indexOf(label))));//填充多边形,并以类别索引做为填充颜色
        }
        if(!mask.empty()) return mask;
    }
}
/**
 * @brief CustomScene::parasJson
 * @param filePath 文件地址
 * @return
 * 解析Json文件
 */
QJsonObject CustomScene::parasJson(QString filePath)
{
    QFile loadFile(filePath);
    if(!loadFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(NULL, tr("Warning!!!"), tr("Could Open Projects Json"));
        return QJsonObject();
    }
    QByteArray allData=loadFile.readAll();
    loadFile.close();


    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData,&json_error));
    if(json_error.error!=QJsonParseError::NoError)
    {
        QMessageBox::information(NULL, tr("Warning!!!"), tr("Read Json file error"));
        return QJsonObject();
    }
    QJsonObject rootObj=jsonDoc.object();
    return rootObj;
}
/**
 * @brief CustomScene::editPointItems
 *  编辑点坐标
 */
void CustomScene::editPointItems()
{

    QList<QPointF> _currentPointsList;
    PointItem *p;
    foreach (QGraphicsItem *item, this->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            p = qgraphicsitem_cast<PointItem *>(item);
            //            p->setCurrentPointList(p->points());


        }
    }



}

void CustomScene::selectBoxItems(bool op)
{
    // selecting box items
    foreach (QGraphicsItem *item, this->items()) {
        qDebug()<<"selectBoxItems ......"<<item;
        if (item->type() == QGraphicsItem::UserType+1) {
            item->setSelected(op);
        }
    }
}

void CustomScene::selectBoxItems(BoxItem *box, bool op)
{
    foreach (QGraphicsItem *item, this->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            item->setSelected(item==qgraphicsitem_cast<QGraphicsItem *>(box));
        }
    }
}
/**
 * @brief CustomScene::selectBoxItems
 * @param point
 * @param op
 * 分割的任务，选取多边形点
 */
void CustomScene::selectBoxItems(PointItem *point, bool op)
{
    foreach (QGraphicsItem *item, this->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            item->setSelected(item==qgraphicsitem_cast<QGraphicsItem *>(point));
        }
    }
}

void CustomScene::selectBoxItems(QList<BoxItem *> *boxList, bool op)
{
    foreach (QGraphicsItem *item, this->items()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            item->setSelected(boxList->contains(qgraphicsitem_cast<BoxItem *>(item)));
        }
    }
}
//检测框的标注
void CustomScene::drawBoxItem(bool op)
{
    _isDrawing = op;
    _isBox=true;
    _isPanning = false;
}

void CustomScene::drawPloyItem(bool op)
{
    _isDrawing = op;

    _isBox=false;
    _isPanning = false;
}

void CustomScene::panImage(bool op)
{
    _isDrawing = false;
    _isPanning = op;
    selectBoxItems(false);
}
/**
 * @brief CustomScene::mousePressEvent
 * @param event
 * 点击鼠标左键的效果
 */
void CustomScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    switch (event->buttons()) {
    case Qt::LeftButton:
        _leftTopPoint = event->scenePos();
        if(_isBox){
            if (_isDrawing && event->modifiers() != Qt::ControlModifier) { // drawing box item

                // no box selected
                int count = 0;
                foreach (QGraphicsItem *item, this->selectedItems()) {
                    if (item->type() == QGraphicsItem::UserType + 1) {
                        item->setSelected(true);
                        if (item->isSelected()) {
                            count++;
                        }
                    }
                }
                if (count <= 0) {
                    QGraphicsScene::mousePressEvent(event);
                    return;
                }

                // selected box do not contain pressed pos
                count = 0;
                foreach (QGraphicsItem *item, this->items()) {
                    if (item->type() == QGraphicsItem::UserType + 1) {
                        if (item->isSelected() && !item->contains(_leftTopPoint)) {
                            item->setSelected(false);
                            count++;
                        }
                    }
                }
                if (count >0) {
                    QGraphicsScene::mousePressEvent(event);
                    return;
                }

                // selected box contain pressed pos
                foreach (QGraphicsItem *item, this->items()) {
                    if ((item->type() == QGraphicsItem::UserType + 1 && item->contains(_leftTopPoint))) {
                        _isMoving = true;
                        QGraphicsScene::mousePressEvent(event);
                        return;
                    }
                }
            }
            else if (event->modifiers() == Qt::ControlModifier) { // selecting multiple box items +ctrl键
                foreach (QGraphicsItem *item, this->items()) {
                    if ((item->type() == QGraphicsItem::UserType + 1 && item->contains(_leftTopPoint))) {
                        item->setSelected(true);
                        break;
                    }
                }
                QGraphicsScene::mousePressEvent(event);
            }
            else {// selecting single box item
                foreach (QGraphicsItem *item, this->items()) {
                    if ((item->type() == QGraphicsItem::UserType + 1 && item->contains(_leftTopPoint))) {
                        selectBoxItems(qgraphicsitem_cast<BoxItem *>(item), true);
                        _isMoving = true;
                        QGraphicsScene::mousePressEvent(event);
                        break;
                    }
                }
            }
        }else{//分割的逻辑
            if(!_leftClick&&_isDrawing) {//双击走这里
                _pointItem = nullptr;
                _leftClick=true;
            }
            if(!_isDrawing){//非标注的时候
                foreach (QGraphicsItem *item, this->items()) {
                    if (item->type() == QGraphicsItem::UserType+1) {
                        PointItem* p = qgraphicsitem_cast<PointItem *>(item);
                        if(p->isContainsPoint(p->points(),_leftTopPoint)||p->isContainsBorderPoint(p->points(),_leftTopPoint)){//当前点是否在该区域内
                            _isPointSelected=true;
                            _currentSelected=p;
                            _currentSelected->setCurrentPoints(_currentSelected->points(),event);//设置当前被选中
                            _oldPointList=_currentSelected->points();//记录旧的点的位置
                            _currentSelected->setItemSelected(false);
                            this->registerItem(_currentSelected);
                            p->setOCRTask(_isOCRTask);//设置是否为OCR功能模块
                            emit typeChange(_currentSelected->typeName());//选中后
                        }else {
                            p->setItemSelected(false);
                            continue;
                        }
                        p->setItemSelected(true);
                    }

                }
            }
        }

    case Qt::RightButton:
        QGraphicsScene::mousePressEvent(event);
        break;
    default:
        break;
    }
    QGraphicsScene::mousePressEvent(event);
}

void CustomScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {//点击左键
        _isMouseMoved = true;
        // add new box item
        if(_isBox){
            if(_isDrawing && (this->selectedItems().count() <= 0 || _boxItem) && !_isMoving && !_isPanning) {
                if(!_boxItem) {
                    _boxItem = new BoxItem(this->sceneRect(), _image->size(), _typeNameList, _typeName);//添加新框
                    this->registerItem(_boxItem);//注册事件
                    this->addItem(_boxItem);
                }
                _boxItem->setSelected(true);
                _rightBottomPoint = event->scenePos();
                qDebug()<<"the rightBottomPoint is ....."<<_rightBottomPoint;
                QRect roi(qMin(_rightBottomPoint.x(), _leftTopPoint.x()),
                          qMin(_rightBottomPoint.y(), _leftTopPoint.y()),
                          qAbs(_rightBottomPoint.x() - _leftTopPoint.x()),
                          qAbs(_rightBottomPoint.y() - _leftTopPoint.y()));
                int l = sceneRect().left(), r = sceneRect().right(),
                        t = sceneRect().top(), b = sceneRect().bottom();
                if (roi.right() < l || roi.left() > r ||
                        roi.top() > b || roi.bottom() < t )
                    return;

                roi.setTop(qMax(t, roi.top()));
                roi.setLeft(qMax(l, roi.left()));
                roi.setBottom(qMin(b, roi.bottom()));
                roi.setRight(qMin(r, roi.right()));
                _boxItem->setRect(roi);
            }
        }
        if (_isMoving) {
            QGraphicsScene::mouseMoveEvent(event);
        }
    }else {
        if(_pointItem&&_isDrawing&&!_isBox) _pointItem->setMovePoint(event->scenePos());//设置鼠标移动描点
        if(!_isDrawing&&!_isBox){
            foreach (QGraphicsItem *item, this->items()) {
                if (item->type() == QGraphicsItem::UserType+1) {
                    PointItem *p = qgraphicsitem_cast<PointItem *>(item);
                    if(p->isContainsPoint(p->points(),event->scenePos())&&!_isPointSelected){//当前点是否在该区域内
                        qDebug()<<"CustomScene::mouseMoveEvent is run........";
                        p->setPoints(p->points());
                        p->update();
                    }

                }
            }
        }
    }
    emit cursorMoved(event->scenePos());//发送移动信号到主界面显示

    if(!(_isDrawing && this->selectedItems().count()<=0))
        QGraphicsScene::mouseMoveEvent(event);
}
/**
 * @brief CustomScene::mouseReleaseEvent
 * @param event
 * 点击鼠标松开事件处理
 */
void CustomScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (_isMouseMoved) {
        if (_isDrawing) {
            if (_boxItem&&_isBox) {
                this->removeItem(_boxItem);
                if ( _boxItem->rect().width() > 5 && (_boxItem->rect().height() > 5 ) ) {
                    QCursor c = Qt::CrossCursor;
                    _boxItem->setOldCursor(c);
                    _undoStack->push(new AddBoxCommand(this, _boxItem));
                }
                _boxItem = nullptr;
                return;
            }
        }
        if (_isMoving) {
            _isMoving = false;
            QGraphicsScene::mouseReleaseEvent(event);
        }
        if(!_isBox&&_currentSelected!=nullptr){
            int status=_currentSelected->getTaskStatus();//获取当前的移动状态
            _undoRedoList.append(_oldPointList);
            QList<QPointF> _newPointList=_currentSelected->points();
            if (status==0) movePoints(_currentSelected,_newPointList,_oldPointList);//当整体移动
            else if(status==1) {//当调整一个点的时候
                _undoRedoList.append(_newPointList);//记录所有移动过的点集
                stretchPoints(_currentSelected,_undoRedoList);
            }
        }
        _isMouseMoved = false;
        return;
    } else {
        if (_boxItemMimeData)
            _clickedPos = event->scenePos();
        if(event->modifiers() != Qt::ControlModifier&&!_isMoving&&!_isBox&&_leftClick&&!_isPanning&&_isDrawing) {//分割的逻辑
            if(!_pointItem) {//当非空的时候
                _pointItem = new PointItem(this->sceneRect(), _image->size(), _typeNameList, _typeName);//添加新标注点
                _pointItem->setOCRTask(_isOCRTask);
                this->registerItem(_pointItem);
                this->addItem(_pointItem);
            }
            _pointItem->setSelected(true);
            QCursor c = Qt::CrossCursor;
            _pointItem->setOldCursor(c);
            _undoStack->push(new AddPointCommand(this, _pointItem));

            if(_leftClick&&event->button()==Qt::LeftButton) {//修复语义分割标注中鼠标右键可以标注的bug
                qDebug()<<"record........left click"<<Qt::RightButton;
                _pointItem->setPoint(event->scenePos());
            }
            _isBox=false;
        }

    }

    QGraphicsScene::mouseReleaseEvent(event);
}
/**
 * @brief CustomScene::mouseDoubleClickEvent
 * @param event
 * 当进行双击操作的时候，首先触发一次mousePressEvent，然后触发一次mouseReleaseEvent
 * 接着才会触发mouseDoubleClickEvent，最后还会触发一次mouseReleaseEvent。
 */
void CustomScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(_isDrawing&&!_isBox){
        if(_pointItem) {
            QCursor c = Qt::CrossCursor;
            _pointItem->setOldCursor(c);//设置鼠标样式
            qDebug()<<"double click is......"<<_pointItem->pointList().size();
            if(_pointItem->pointList().size()<3)
            {
                _leftClick=true;//防止左键
            }else{
                _pointItem->setOCRTask(_isOCRTask);
                _pointItem->setClosePoint();
                _pointItem=nullptr;
                _leftClick=false;
            }
        }

    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

void CustomScene::keyPressEvent(QKeyEvent *keyEvent)
{
    QList<Qt::Key> keyList={Qt::Key_0,Qt::Key_1,Qt::Key_2,Qt::Key_3,Qt::Key_4,Qt::Key_5,Qt::Key_6,Qt::Key_7,Qt::Key_8,Qt::Key_9};
    if(keyEvent->key() == Qt::Key_Delete) {
        deleteBoxItems();
    } else if(keyEvent->key() == Qt::Key_A && keyEvent->modifiers() == Qt::ControlModifier) {
        selectBoxItems(true);
    }else if(keyList.contains(Qt::Key(keyEvent->key()))){//相应键盘按键事件
        int _typeIndex=keyEvent->key()-48;
        if(_typeIndex<_typeNameList.length()-1) _typeName=_typeNameList.at(_typeIndex);
        if (_isBox) changeBoxTypeName(_typeName);
        else changePointTypeName(_typeName);
        emit typeChange(_typeIndex);
    }else {
        QGraphicsScene::keyPressEvent(keyEvent);
    }
}

void CustomScene::keyReleaseEvent(QKeyEvent *keyEvent)
{
    QGraphicsScene::keyReleaseEvent(keyEvent);
}
/**
 * @brief CustomScene::changeBoxTypeIndex
 * @param index
 *
 */
void CustomScene::changeBoxTypeName(QString name)
{
    _typeName = name;
    QList<BoxItem *> *boxList = new QList<BoxItem *>();
    foreach (QGraphicsItem *item, this->selectedItems()) {
        if (item->type() == QGraphicsItem::UserType+1) {
            BoxItem *b = qgraphicsitem_cast<BoxItem *>(item);
            boxList->append(b);
        }
    }

    if (boxList->count() > 0) {
        _undoStack->push(new SetTargetTypeCommand(this, boxList, name));
    }
    delete boxList;
}
/**
 * @brief CustomScene::changePointTypeName
 * @param name
 * 下拉框改变调用的事件
 */
void CustomScene::changePointTypeName(QString name)
{
    _typeName = name;
    if(_currentSelected&&!_isDrawing){
        qDebug()<<"changePointTypeName is run............";
        _currentSelected->setMaskPointList(_currentSelected->points());
        _currentSelected->setTypeName(_typeName);
        emit typeChange(_currentSelected->typeName());//选中后
    }

}
/**
  改变box的标签值
 * @brief CustomScene::changePointLabelName
 * @param name
 */
void CustomScene::changePointLabelName(QString name)
{
    _typeName=name;
    qDebug()<<"customsce.....get name......."<<_typeName;
    emit labelChange(_typeName);

}
void CustomScene::clearUndoStack()
{
    _undoStack->clear();

}
void CustomScene::moveBox(QRectF newRect, QRectF oldRect)
{
    BoxItem *item = reinterpret_cast<BoxItem *>(QObject::sender());
    if (item != nullptr) {
        _undoStack->push(new MoveBoxCommand(this, item, newRect, oldRect));
    }
}
/**
 * @brief CustomScene::stretchPoints
 * @param pointItem 当前移动的框
 * @param newList 新的点集
 * @param index 移动点的坐标
 * 移动坐标点的redo和undo操作
 */
void CustomScene::stretchPoints(PointItem *pointItem,QList<QList<QPointF>> redoUndoList)
{
    if (pointItem != nullptr) {
        _undoStack->push(new StretchPointsCommand(this, pointItem, redoUndoList));
    }
}
/**
 * 图像增强
 * @brief CustomScene::augMentPics
 * @param filePath 图片地址
 */
cv::Mat CustomScene::augMentPics(Mat img, int angle)
{
    cv::Point2f center(img.cols/2,img.rows/2);
    cv::Mat M=cv::getRotationMatrix2D(center,-angle,1);
    cv::Mat dst;
    cv::warpAffine(img,dst,M,cv::Size(img.cols,img.rows));
    return dst;


}
/**
 * @brief CustomScene::voc2YOLO 将VOC标记的格式转换为yolo标记的格式
 * @param id 标签编号
 * @param width 图片的宽度
 * @param height 图片的高度
 * @param point_L_U 左上坐标
 * @param point_R_L 右下坐标
 * @return
 */
qreal *CustomScene::voc2YOLO(int width, int height, Point point_L_U, Point point_R_L)
{
    qreal dw=1.0/width;
    qreal dh=1.0/height;
    int xMin=point_L_U.x;
    int xMax=point_R_L.x;
    int yMin=point_L_U.y;
    int yMax=point_R_L.y;
    qreal x=(xMin+xMax)/2.0-1;
    qreal y=(yMin+yMax)/2.0-1;
    int w=abs(xMax-xMin);
    int h=abs(yMax-yMin);
    qreal _x=x*dw;
    qreal _w=w*dw;
    qreal _y=y*dh;
    qreal _h=h*dh;
    qreal _result[]={_x,_y,_w,_h};
    qreal* temp = new qreal[4];
    for ( int i =0; i < 4; i++) temp[i] = (qreal)_result[i];//C++要求必须这样写
    return temp;
}
/**
 * @brief CustomScene::YOLO2VOC 将YOLO真实的坐标转换为VOC标记格式
 * @param w 实际的宽度
 * @param h 实际的高度
 * @param x 实际中心点x坐标
 * @param y 实际中心点y坐标
 * @return
 */
int *CustomScene::YOLO2VOC(int id,qreal w, qreal h, qreal x, qreal y)
{
    int* temp=new int[4];
    int xMin=int(x-w/2);
    int yMin=int(y-h/2);
    int xMax=int(x+w/2);
    int yMax=int(y+h/2);
    int _result[]={id,xMin,yMin,xMax,yMax};
    for ( int i =0; i < 5; i++) temp[i] = (qreal)_result[i];//C++要求必须这样写
    return temp;

}
/**
 * 生成旋转后的图片和标记框
 * @brief CustomScene::generateImgCoor
 * @param augPicBasePath 图片保存的基路径
 * @param rote_degree 旋转的角度
 * @param isAug 是否增强
 * @param img 返回旋转后的图片
 * @return
 */
void CustomScene::generateImgCoor(QString augPicBasePath, int rote_degree, bool isAug, cv::Mat img,QList<int*> voc_lst)
{
    QString augPicPath=augPicBasePath+".png";
    QString augPicAnnoPath=augPicBasePath+".txt";
    int a=img.cols/2;//围绕中心点进行旋转
    int b=img.rows/2;//围绕中心点进行旋转
    int width=img.cols;//图片的宽度
    int height=img.rows;//图片的高度
    QList<int*> _voc_lst;
    QList<QMap<int,QPair<cv::Point,cv::Point>>> yoloLst;
    float theta=rote_degree*CV_PI/180.0;//计算旋转后的角度
    cv::Mat augPic=augMentPics(img,rote_degree);//旋转图片
    qDebug()<<"the length is ...."<<voc_lst<<"---------"<<voc_lst.length();
    QFile yolofile(augPicAnnoPath);
    foreach(int* voc,voc_lst){
        cv::Point point_L_U;//左上坐标
        cv::Point point_R_L;//右下坐标
        point_L_U.x=(voc[1]-a)*cos(theta)-(voc[2]-b)*sin(theta)+a;
        point_L_U.y=(voc[1]-a)*sin(theta)+(voc[2]-b)*cos(theta)+b;
        point_R_L.x=(voc[3]-a)*cos(theta)-(voc[4]-b)*sin(theta)+a;
        point_R_L.y=(voc[3]-a)*sin(theta)+(voc[4]-b)*cos(theta)+b;
        if(point_L_U.x<0||point_L_U.y<0||point_R_L.x<0||point_R_L.y<0)
        {
            isAug=false;//旋转超过图片的范围则不适合做旋转增强直接摄取,进行下一增强
            return;
        }
        _voc_lst.push_back(voc);
        QPair<cv::Point,cv::Point> _temPair(point_L_U,point_R_L);
        QMap<int,QPair<cv::Point,cv::Point>> _temp;
        _temp.insert(voc[0],_temPair);
        yoloLst.append(_temp);
    }
    qDebug()<<"the length is ...."<<yoloLst.length()<<"---------"<<voc_lst.length();
    if(yoloLst.length()==voc_lst.length())
    {
        yolofile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&yolofile);
        for(auto lst:yoloLst)
        {
            qDebug()<<"the qMap is ...."<<lst.first().first.x;
            //            qreal *yoloAnno=voc2YOLO(width,height,lst.first().first,lst.first().second);
            qreal *yoloAnno=voc2YOLO(augPic.cols,augPic.rows,lst.first().first,lst.first().second);
            QString s;
            s.sprintf("%d %f %f %f %f\n",
                      lst.keys()[0],
                    yoloAnno[0],yoloAnno[1],yoloAnno[2],yoloAnno[3]);
            out << s;

        }
        yolofile.close();
        cv::imwrite(augPicPath.toLocal8Bit().toStdString(),augPic);

    }

}
/**
 * @brief CustomScene::gvAugMent
 * @param gvDir 所需增强的文件夹
 */
void CustomScene::gvAugMent(QString gvDir)
{
    QDir dir(gvDir);
    if(!dir.exists()){
        return;
    }
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::DirsFirst);//文件夹排在前面
    QFileInfoList list=dir.entryInfoList();//获取本文件内所有文件信息
    int fileCount=list.count();
    if(fileCount<1){
        return;
    }
    for(int i=0;i<fileCount;i++){
        QFileInfo fileInfo=list[i];
        QString currentDirPath=fileInfo.path();
        if (fileInfo.suffix()=="png"||fileInfo.suffix()=="jpg"||fileInfo.suffix()=="jpeg"||fileInfo.suffix()=="bmp"
                ||fileInfo.suffix()=="gif"){
            //            QString fileName=fileInfo.baseName();
            QString _fileName=fileInfo.fileName();
            QString fileName=_fileName.left(_fileName.lastIndexOf('.'));
            int gvIndex=(int)fileInfo.baseName().lastIndexOf("_");
            int gv=fileName.right(fileName.length()-gvIndex-1).toInt();//从文件名中读取gv差值
            if(gv==0){
                return;
            }
            qDebug()<<"the gv is......."<<gv;
            QString filePath=fileInfo.absoluteFilePath();
            QString originYOLOAnno=currentDirPath+"/"+fileName+".txt";
            cv::Mat img=cv::imread(filePath.toLocal8Bit().toStdString(),0);
            cv::Mat dst;
            int width=img.cols;//图片的宽度
            int height=img.rows;//图片的高度
            QString yoloAnnoFile=currentDirPath+"/"+fileName+".txt";
            QFile file(yoloAnnoFile);
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            qreal x,y,w,h;//真实的中心点坐标和宽高
            QString augPicGVPath=currentDirPath+"/"+fileName+"_"+"gv.png";
            QString augPicGVAnnoPath=currentDirPath+"/"+fileName+"_"+"gv.txt";
            dst=img;
            int deltaGV=gv-23;//求缺陷和背景的gv差值和23的关系
            qDebug()<<"the result is......."<<deltaGV;
            QTextStream in(&file);
            int id;//标签编号
            int index;
            int *voc;
            QList<int*> voc_lst;
            while (!in.atEnd()) {//读取标注文件
                QStringList info = in.readLine().split(" ");
                if(info.size() >= 5) {
                    index = info.at(0).toInt();
                    if(index==-1 || index>_typeNameList.count()){
                        QMessageBox::critical(0 , "Alert" , "标签文件内容有误", QMessageBox::Ok | QMessageBox::Default , QMessageBox::Cancel | QMessageBox::Escape , 	0 );
                        return;
                    }
                    id=info.at(0).toInt();
                    w = info.at(3).toFloat() * width;
                    h = info.at(4).toFloat() * height;
                    x = info.at(1).toFloat() * width;
                    y = info.at(2).toFloat() * height;
                    voc=YOLO2VOC(id,w,h,x,y);
                    voc_lst.push_back(voc);//将标注结果放到list中
                }
            }
            file.close();
            if(deltaGV<0){//增加gv值改变的逻辑代码
                for(int *_voc:voc_lst){//遍历所有的标记坐标
                    int height1=_voc[4]-_voc[2];//求高
                    int width2=_voc[3]-_voc[1];//求宽
                    int x0=_voc[1];//起始点x坐标
                    int y0=_voc[2];//起始点y坐标
                    uchar* y_data;
                    for(size_t i=y0;i<height1+y0;i++){
                        y_data = img.ptr<uchar>(i);//y行的首地址
                        for (size_t x = x0; x < width2+x0; x++)
                        {
                            //                            int value = y_data[x];
                            //                            dst.at<uchar>(x) = y_data[x]+deltaGV;
                            y_data[x]=y_data[x]+deltaGV;
                        }
                    }


                }
            }else{
                for(int *_voc:voc_lst){//遍历所有的标记坐标
                    int height1=_voc[4]-_voc[2];//求高
                    int width2=_voc[3]-_voc[1];//求宽
                    int x0=_voc[1];//起始点x坐标
                    int y0=_voc[2];//起始点y坐标
                    uchar* y_data;
                    for(size_t i=y0;i<height1+y0;i++){
                        y_data = img.ptr<uchar>(i);//y行的首地址
                        for (size_t x = x0; x < width2+x0; x++)
                        {
                            //                             dst.at<uchar>(x) = y_data[x]-deltaGV;
                            y_data[x]=y_data[x]-deltaGV;
                        }
                    }
                    //                    float v = img.at<uchar>(x0+j, y0+i)+deltaGV;

                }
            }
            QFile::copy(originYOLOAnno,augPicGVAnnoPath);//复制标签文件
            cv::imwrite(augPicGVPath.toLocal8Bit().toStdString(),img);
        }

    }
}


/**
 * @brief CustomScene::movePoints
 * @param pointItem 当前复选框
 * @param newList 新的点集
 * @param oldList 旧的点集
 */
void CustomScene::movePoints(PointItem *pointItem,QList<QPointF> newList,QList<QPointF> oldList)
{
    if(pointItem!=nullptr){
        _undoStack->push(new MovePointsCommand(this, pointItem, newList, oldList));
    }
}

/**
 * @brief CustomScene::closePoint
 * 实现封口的槽函数
 */
void CustomScene::closePoint(){
    if(_pointItem) _pointItem=nullptr;
    if(_undoStack) _undoStack->clear();//清空撤销操作
}
void CustomScene::setCurentPointsList()
{

}
/**
 * @brief CustomScene::isBoxTask
 * @return
 * 定义是否为标注任务
 */
void CustomScene::setBoxTask(bool isBoxTask){
    _isBoxTask=isBoxTask;
}

void CustomScene::setClassTask(bool isClassTask)
{
    _isClassTask=isClassTask;
}
void CustomScene::setSegTask(bool isSegTask){
    _isSegTask=isSegTask;
}

void CustomScene::setOCRTask(bool isOCRTask)
{
    _isOCRTask=isOCRTask;
}
void CustomScene::copy()
{
    if (selectedItems().count() <=0 )
        return;

    if (_boxItemMimeData) {
        delete _boxItemMimeData;
    }
    _boxItemMimeData = new BoxItemMimeData(selectedItems());
    QApplication::clipboard()->setMimeData(_boxItemMimeData);

    _pastePos.clear();
    for (int i=0; i<selectedItems().count(); i++)
        _pastePos.append(QPointF(0,0));
    _clickedPos = QPointF(0,0);
}

void CustomScene::paste()
{
    QMimeData *m = const_cast<QMimeData *>(QApplication::clipboard()->mimeData()) ;
    BoxItemMimeData *data = dynamic_cast<BoxItemMimeData*>(m);
    if (data){
        clearSelection();

        QList<QPointF> offset;
        QList<QRectF> itemRects;
        QRectF unitedRect(0,0,0,0);

        for (int i=0; i<data->items().count(); i++) {
            BoxItem *b = qgraphicsitem_cast<BoxItem*>(data->items().at(i));
            itemRects.append(b->rect());
            unitedRect = unitedRect.united(b->rect());
        }
        if (!unitedRect.isNull()) {
            for (int i=0; i<itemRects.count(); i++) {
                offset.append(itemRects[i].topLeft() - unitedRect.topLeft());
            }
        }

        for (int i=0; i<_pastePos.count(); i++) {
            if (_pastePos[i].isNull()) {
                _pastePos[i] = QPointF(itemRects[i].x(), itemRects[i].y());
            }
        }
        if (!_clickedPos.isNull()) {
            unitedRect.moveCenter(_clickedPos);
            for (int i=0; i<_pastePos.count(); i++) {
                _pastePos[i] = unitedRect.topLeft() + offset[i];
            }
            _clickedPos = QPointF(0,0);
        }
        itemRects.clear();
        offset.clear();
        int count = 0;
        QList<BoxItem*> *copyList = new QList<BoxItem *>();
        foreach (QGraphicsItem* item, data->items()) {
            if (item->type() == QGraphicsItem::UserType + 1) {
                BoxItem *b = qgraphicsitem_cast<BoxItem*>(item);
                if (_clickedPos.isNull())
                    _pastePos[count] += QPointF(10, 10);
                QRectF rect(_pastePos[count].x(), _pastePos[count].y(), b->rect().width(), b->rect().height());
                BoxItem *copy = b->copy();
                copy->setRect(rect);
                copy->setSelected(true);
                this->registerItem(copy);
                copyList->append(copy);
                count++;
            }
        }
        if (copyList->count() > 0) {
            _undoStack->push(new AddBoxCommand(this, copyList));
        }
    }
}
void CustomScene::delete_box()
{
    deleteBoxItems();

}
void CustomScene::delete_ploy()
{
    QList<QPointF> _selectedList;
    if(_currentSelected&&!_isDrawing){
        qDebug()<<"delet_ploy is run............"<<_currentSelected->points();
        _selectedList=_currentSelected->points();
        this->removeItem(_currentSelected);
        if(_isOCRTask) emit labelChange("");
    }

    //undo处理
    if (_currentSelected) {
        _undoStack->push(new RemovePointsCommand(this, _currentSelected));
        //        QApplication::setOverrideCursor(qgraphicsitem_cast<BoxItem *>(boxList.at(0))->oldCursor());
    }
    //    boxList.clear();
}
void CustomScene::editPoints()
{
    _isDrawing=false;
    editPointItems();
}

void CustomScene::cut()
{
    QList<QGraphicsItem*> copyList;
    foreach (QGraphicsItem *item , selectedItems()) {
        if (item->type() == QGraphicsItem::UserType + 1) {
            BoxItem *b = qgraphicsitem_cast<BoxItem*>(item);
            BoxItem *copy = b->copy();
            copyList.append(copy);
        }
    }
    if (copyList.count() > 0){
        deleteBoxItems();
        if (_boxItemMimeData) {
            delete _boxItemMimeData;
        }
        _boxItemMimeData = new BoxItemMimeData(copyList);
        QApplication::clipboard()->setMimeData(_boxItemMimeData);

        _pastePos.clear();
        for (int i=0; i<copyList.count(); i++)
            _pastePos.append(QPointF(0,0));
        _clickedPos = QPointF(0,0);
    }
    copyList.clear();
}

void CustomScene::clipboardDataChanged()
{
    //    QObject::sender()
    //   pasteAction->setEnabled(true);
}
