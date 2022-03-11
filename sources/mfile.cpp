#include "mfile.h"

mFile::mFile(QObject *parent) : QObject(parent)
{
	

}
QString mFile::m_curIniPath = "";
QString mFile::logname = "";

mFile* mFile::mfileptr = nullptr;
mFile *mFile::GetInstance()
{
    if(mfileptr == nullptr)
    {
        mfileptr = new mFile;
    }
    return mfileptr;
}

QString mFile::ReadIni(QString path, QString key)
{
    if(path.isEmpty() || !path.contains(".ini"))
    {
        return "";
    }
    QSettings ini(path, QSettings::IniFormat);
    QString result = ini.value(key).toString();
    return result;
}



bool mFile::SetCurIni(QString path)
{
	if (!FileExist(path))
		CreateFile(path);
	m_curIniPath = path;
	return true;
}

QVariant mFile::ReadIniByKey(QString section, QString key)
{
	if (m_curIniPath == "")
		return "";
	QSettings ini(m_curIniPath, QSettings::IniFormat);
	QString serchKey = QString("/%1/%2").arg(section).arg(key);
	
	return ini.value(serchKey);
}

void mFile::WriteIni(QString path, QString key, QString value)
{
    if(path.isEmpty() || !path.contains(".ini") || key.isEmpty() || value.isEmpty())
    {
        return;
    }
    QSettings ini(path,QSettings::IniFormat);
    ini.setValue(key,value);
}

void mFile::WriteIni(QString path, QStringList keys, QStringList values)
{
    if(path.isEmpty() || keys.count()!=values.count())
    {
        return;
    }
    for(int i = 0;i<keys.count();i++)
    {
        WriteIni(path,keys[i],values[i]);
    }
}

bool mFile::WriteIniQVariant(QString section, QString key, QVariant value)
{
	if (!FileExist(m_curIniPath))
		return false;
	QString writeKey = QString("/%1/%2").arg(section).arg(key);
	QSettings ini(m_curIniPath, QSettings::IniFormat);
	ini.setValue(writeKey, value);
	return true;
}

QString mFile::ReadRegedit(QString path, QString key)
{
    if(path.isEmpty())
    {
        return "";
    }
    QSettings regedit("LOG",path);
    QString result = regedit.value(key).toString();
    return result;
}

QStringList mFile::ReadRegedit(QString path, QStringList keys)
{
    QStringList result;
    if(path.isEmpty())
    {
        return result;
    }
    for(int i = 0;i<keys.count();i++)
    {
        result.append(ReadRegedit(path,keys[i]));
    }
    return result;
}

void mFile::WriteRegedit(QString path, QString key, QString value)
{
    if(path.isEmpty())
    {
        return;
    }
    QSettings regedit("LOG",path);
    regedit.setValue(key,value);
}

void mFile::WriteRegedit(QString path, QStringList keys, QStringList values)
{
    if(path.isEmpty() || keys.count()!=values.count())
    {
        return;
    }
    for(int i = 0;i<keys.count();i++)
    {
        WriteRegedit(path,keys[i],values[i]);
    }
}


bool mFile::ReadFileGetLineContent(QString filename, QStringList& strList)
{
	QFile csvFile(filename);
	if (csvFile.open(QIODevice::ReadWrite))
	{
		QTextStream stream(&csvFile);
		while (!stream.atEnd())
		{
			strList.append(stream.readLine());
		}
		csvFile.close();
	}
	else
		return false;
	return true;
}

void mFile::WriteToCsv(QString filename, QStringList data)
{
    QFile csvFile(filename);
    if (csvFile.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        QString d2 = "";
        for(int i = 0; i < data.count(); ++i)
        {
            d2 += data[i];
            if (i < data.count()-1){
                d2 += ",";
            }
        }
        d2 += "\r\n";
        csvFile.write(d2.toLocal8Bit());
        csvFile.close();
    }
}

QStringList mFile::ReadFromCsv(QString filename)
{
    QStringList result;
    QFile csvFile(filename);
    if (csvFile.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&csvFile);
        while (!stream.atEnd())
        {
           result.append(stream.readLine());
        }
        csvFile.close();
    }
    return result;
}

void mFile::WriteToCsv2(QString filename, QStringList *data, int len)
{
    QFile csvFile(filename);
    if (csvFile.open(QIODevice::ReadWrite))
    {
        QString d2 = "";
        for (int var = 0; var < len; ++var) {
            for(int i = 0; i < data[0].count(); ++i)
            {
                d2 += data[var][i];
                if (i < data[0].count()-1){
                    d2 += ",";
                }
            }
            d2 += "\r\n";
        }
        csvFile.write(d2.toLocal8Bit());
        csvFile.close();
    }
}

void mFile::WriteToTxt(QString filename, QString message)
{
    QFile log(filename);
    if(log.open(QIODevice::ReadWrite | QIODevice::Append))
    {
//        log.write("\r\n");
        log.write(message.toLatin1());
        log.close();
    }
}

QString mFile::ReadFromTxt(QString filename)
{
    QString data = "";
    QFile log(filename);
    if(log.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        data = log.readAll();
        log.close();
    }
    return data;
}

bool mFile::CsvToTxt(QString csvpath, QString txtpath)
{
    if(csvpath.isEmpty() || txtpath.isEmpty() || !FileExist(csvpath))
    {
        return false;
    }
    QStringList ret = ReadFromCsv(csvpath);
    if(FileExist(txtpath))
    {
        DeleteFile(txtpath);
    }
    CreateFile(txtpath);
    QFile txt(txtpath);
    if(txt.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        for(int i = 0;i<ret.count();i++)
        {
            txt.write((ret[i] + "\r\n").toLatin1());
        }
        txt.close();
        return true;
    }
    return false;
}

bool mFile::TxtToCsv(QString txtpath, QString csvpath)
{
    if(csvpath.isEmpty() || txtpath.isEmpty() || !FileExist(txtpath))
    {
        return false;
    }
    QStringList source;
    QFile txt(txtpath);
    if (txt.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&txt);
        while (!stream.atEnd())
        {
            source.append(stream.readLine());
        }
        txt.close();
    }
    if(FileExist(csvpath))
    {
        DeleteFile(csvpath);
    }
    CreateFile(csvpath);
    for(int i=0;i<source.count();i++)
    {
        source[i].chop(1);
        source[i] += "\r\n";
    }
    WriteToCsv(csvpath,source);
    return true;
}

bool mFile::DirExist(QString fullpath)
{
    if(fullpath.isEmpty())
    {
        return false;
    }
    QDir logdir;
    if(logdir.exists(fullpath))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool mFile::FileExist(QString fullpath)
{
    if(fullpath.isEmpty())
    {
        return false;
    }
    QFileInfo fileInfo(fullpath);
    if(fileInfo.isFile())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void mFile::CreateDir(QString fullpath)
{
    if(fullpath.isEmpty())
    {
        return;
    }
    if(!DirExist(fullpath))
    {
        QDir dir;
        dir.mkpath(fullpath);
    }
}

void mFile::CreateFile(QString fullpath)
{
    if(fullpath.isEmpty())
    {
        return;
    }
    if(!FileExist(fullpath))
    {
        QFile file;
        file.setFileName(fullpath);
        file.open(QIODevice::ReadWrite);
        file.close();
    }
}

void mFile::DeleteDir(QString fullpath)
{
    if(fullpath.isEmpty())
    {
        return;
    }
    QDir dir(fullpath);
    if(!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (QFileInfo file, fileList)
    {
        if (file.isFile())
        {
            file.dir().remove(file.fileName());
        }
        else
        {
            DeleteDir(file.absoluteFilePath());
        }
    }
    dir.rmpath(dir.absolutePath());
}

void mFile::DeleteFile(QString fullpath)
{
    if(fullpath.isEmpty())
    {
        return;
    }
    if(FileExist(fullpath))
    {
        QFile file(fullpath);
        file.remove();
    }
}


