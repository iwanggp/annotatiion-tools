#ifndef MFILE_H
#define MFILE_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QTextStream>
#include <QDateTime>
#include <QTextCodec>
//#define FILE_NAEE_CFG ".\\..\\x64\\machine.ini" //依据x64 结构做的
#define FILE_NAEE_PATHMANAGER	(QCoreApplication::applicationDirPath()+"/"+"pathManager.ini") //依据x64 结构做的
//#define CONFIG_PATH "machine.ini"; 

class mFile : public QObject
{
    Q_OBJECT
public:
    //explicit mFile(QObject *parent = nullptr);
    static mFile* GetInstance();
signals:
public slots:

public:
    //  INI文件
	static QString ReadIni(QString path, QString key);
	
	static bool SetCurIni(QString path);
    QVariant ReadIniByKey(QString section,QString key);
    bool WriteIniQVariant(QString section, QString key, QVariant value);


    //static QString ReadIni(QString path,QString key);
    //static QStringList ReadIni(QString path,QStringList keys);
    static void WriteIni(QString path,QString key,QString value);
    static void WriteIni(QString path,QStringList keys,QStringList values);
    //  注册表
    static QString ReadRegedit(QString path,QString key);
    static QStringList ReadRegedit(QString path,QStringList keys);
    static void WriteRegedit(QString path,QString key,QString value);
    static void WriteRegedit(QString path,QStringList keys,QStringList values);
	//文本读行
	static bool ReadFileGetLineContent(QString filename,QStringList& strList);

    //  CSV文件
    static void WriteToCsv(QString filename, QStringList data);
    static QStringList ReadFromCsv(QString filename);
    static void WriteToCsv2(QString filename, QStringList *data, int len);
    //  LOG文件
    static void WriteToTxt(QString filename, QString message);
    static QString ReadFromTxt(QString filename);
    //  转换
    static bool CsvToTxt(QString csvpath,QString txtpath);
    static bool TxtToCsv(QString txtpath,QString csvpath);
    //  文件
    static bool DirExist(QString fullpath);
    static bool FileExist(QString fullpath);
    static void CreateDir(QString fullpath);
    static void CreateFile(QString fullpath);
    static void DeleteDir(QString fullpath);
    static void DeleteFile(QString fullpath);
    static bool ChangeFileName(QString oldPath,QString newPath)
    {
        if (!FileExist(oldPath))
            return false;
        return QFile::rename(oldPath,newPath);
    }

    static QString logname;
	static QString m_curIniPath;
private:
    explicit mFile(QObject *parent = nullptr);
    static mFile* mfileptr;
};

#endif // MFILE_H
