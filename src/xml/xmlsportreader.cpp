#include "xmlsportreader.h"

XkorXmlSportReader::XkorXmlSportReader(QString filename)
{
    m_filename = filename;
    QFile * f = new QFile(m_filename);
	if(!f->exists())
	{
        raiseError(QObject::tr("File ‘%1’ not found by XkorXmlSportReader::XkorXmlSportReader(QString)").arg(m_filename));
		return;
	}
	f->open(QIODevice::ReadOnly);
	setDevice(f);

	while(!atEnd())
	{
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("sport")) == 0 && attributes().value("version").compare(QString::fromUtf8("0.3")) == 0)
				readFile();
			else
                raiseError(QObject::tr("The file ‘%1’ is not an xkoranate version 0.3 sport.").arg(m_filename));
		}
		readNext();
	}

	if(hasError())
		qDebug() << error();

	delete f;
}

QString XkorXmlSportReader::error() const
{
	QXmlStreamReader::Error err = QXmlStreamReader::error();
	QString rval;
	switch(err)
	{
		case 0:
			return QString(); // no error
		case 1:
			rval = QObject::tr("An unexpected element was found.");
			break;
		case 3:
			rval = QObject::tr("The input file was not well-formed XML.");
			break;
		case 4:
			rval = QObject::tr("The document ended unexpectedly.");
			break;
		case 2:
			rval = errorString();
			break;
	}
    rval += QObject::tr("\nError occured at line %1, column %2 of ‘%3’.").arg(QString::number(lineNumber())).arg(QString::number(columnNumber())).arg(m_filename);
	return rval;
}

QMap<double, double> XkorXmlSportReader::readDataPoints()
{
	QMap<double, double> rval;

	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("datapoint")) == 0)
			{
				double pos = attributes().value("pos").toString().toDouble();
				rval.insert(pos, readDouble());
			}
			else
				readUnknownElement();
		}
	}
	return rval;
}

double XkorXmlSportReader::readDouble()
{
	return readElementText().toDouble();
}

void XkorXmlSportReader::readFile()
{
	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("name")) == 0)
				m_sport.setName(readString());
            else if(name().compare(QString::fromUtf8("alphabetizedName")) == 0)
				m_sport.setAlphabetizedName(readString());
            else if(name().compare(QString::fromUtf8("discipline")) == 0)
				m_sport.setDiscipline(readString());
            else if(name().compare(QString::fromUtf8("event")) == 0)
				m_sport.setEvent(readString());
            else if(name().compare(QString::fromUtf8("scorinator")) == 0)
				m_sport.setScorinator(readString());
            else if(name().compare(QString::fromUtf8("paradigm")) == 0)
				m_sport.setParadigm(readString());
            else if(name().compare(QString::fromUtf8("paradigmOptions")) == 0)
			{
				QHash<QString, QVariant> options = readOptions();
				m_sport.setParadigmOptions(options);
			}
            else if(name().compare(QString::fromUtf8("dataPoints")) == 0)
			{
				QString name = attributes().value("name").toString();
				QMap<double, double> dataPoints = readDataPoints();
				m_sport.addDataPoints(name, dataPoints);
			}
			else
				readUnknownElement();
		}
	}
}

int XkorXmlSportReader::readInt()
{
	return readElementText().toInt();
}

QList<QVariant> XkorXmlSportReader::readList()
{
	QList<QVariant> rval;
	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("double")) == 0)
			{
				double value = readDouble();
				rval.append(value);
			}
            else if(name().compare(QString::fromUtf8("int")) == 0)
			{
				int value = readInt();
				rval.append(value);
			}
            else if(name().compare(QString::fromUtf8("list")) == 0)
			{
				QList<QVariant> value = readList();
				rval.append(QVariant(value)); // explicit conversion to QVariant needed to avoid calling append(QList<>)
			}
            else if(name().compare(QString::fromUtf8("string")) == 0)
			{
				QString value = readString();
				rval.append(value);
			}
			else
				readUnknownElement();
		}
	}
	// readNext(); // skip end element
	return rval;
}

QHash<QString, QVariant> XkorXmlSportReader::readOptions()
{
	QHash<QString, QVariant> rval;
	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
			QString type = attributes().value("type").toString();
            if(name().compare(QString::fromUtf8("double")) == 0)
			{
				double value = readDouble();
				rval.insert(type, value);
			}
            else if(name().compare(QString::fromUtf8("int")) == 0)
			{
				int value = readInt();
				rval.insert(type, value);
			}
            else if(name().compare(QString::fromUtf8("list")) == 0)
			{
				QList<QVariant> value = readList();
				rval.insert(type, value);
			}
            else if(name().compare(QString::fromUtf8("string")) == 0)
			{
				QString value = readString();
				rval.insert(type, value);
			}
			else
				readUnknownElement();
		}
	}
	return rval;
}

QString XkorXmlSportReader::readString()
{
	return readElementText();
}

void XkorXmlSportReader::readUnknownElement()
{
	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
			readUnknownElement();
	}
}

