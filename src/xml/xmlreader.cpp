#include "xmlreader.h"


XkorXmlReader::XkorXmlReader(QString filename)
{
	m_rpList = 0;

	QFile * f = new QFile(filename);
	if(!f->exists())
	{
		raiseError(QObject::tr("File ‘%1’ not found by XkorXmlReader::XkorXmlReader(QString)").arg(filename));
		return;
	}
	f->open(QIODevice::ReadOnly);
	setDevice(f);

	while(!atEnd())
	{
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("scorinationFile")) == 0 && attributes().value("version").compare(QString::fromUtf8("0.3.3")))
				readFile();
			else
				raiseError(QObject::tr("This file is not an xkoranate version 0.3 file."));
		}
		readNext();
	}

	if(hasError())
		qDebug() << error();

	delete f;
}

QString XkorXmlReader::error() const
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
	rval += QObject::tr("\nError occured at line %1, column %2.").arg(QString::number(lineNumber())).arg(QString::number(columnNumber()));
	return rval;
}

double XkorXmlReader::readDouble()
{
	return readElementText().toDouble();
}

void XkorXmlReader::readEvent()
{
	XkorEvent * event = new XkorEvent;
	QUuid id = QUuid(attributes().value("id").toString());
	event->setName(attributes().value("name").toString());
	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("sport")) == 0)
				event->setSport(readString());
            else if(name().compare(QString::fromUtf8("competition")) == 0)
				event->setCompetition(readString());
            else if(name().compare(QString::fromUtf8("paradigmOptions")) == 0)
				event->setParadigmOptions(readOptions());
            else if(name().compare(QString::fromUtf8("competitionOptions")) == 0)
				event->setCompetitionOptions(readOptions());
            else if(name().compare(QString::fromUtf8("results")) == 0)
				event->setResults(readResults());
            else if(name().compare(QString::fromUtf8("signupList")) == 0)
				event->setSignupList(readEventSignupList());
            else if(name().compare(QString::fromUtf8("group")) == 0)
				event->addGroup(readGroup());
			else
				readUnknownElement();
		}
	}
	m_events.append(qMakePair(id, event));
}

XkorSignupList XkorXmlReader::readEventSignupList()
{
	XkorSignupList rval;

	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("maxRank")) == 0)
				rval.setMaxRank(readDouble());
            else if(name().compare(QString::fromUtf8("minRank")) == 0)
				rval.setMinRank(readDouble());
            else if(name().compare(QString::fromUtf8("signup")) == 0)
				rval.addAthlete(readSignup());
			else
				readUnknownElement();
		}
	}
	return rval;
}

void XkorXmlReader::readFile()
{
	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("rpList")) == 0)
				readRPList();
            else if(name().compare(QString::fromUtf8("event")) == 0)
				readEvent();
			else
				readUnknownElement();
		}
	}
}

XkorGroup XkorXmlReader::readGroup()
{
	XkorGroup rval;
	rval.name = attributes().value("name").toString();
	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("signup")) == 0)
                rval.athletes.append(QUuid::fromString(readString()));
			else
				readUnknownElement();
		}
	}
	return rval;
}

int XkorXmlReader::readInt()
{
	return readElementText().toInt();
}

QList<QVariant> XkorXmlReader::readList()
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

QPair<QString, QHash<QString, double> > XkorXmlReader::readNation()
{
	QPair<QString, QHash<QString, double> > rval;
	rval.first = attributes().value("name").toString();

	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("property")) == 0)
			{
				QString type = attributes().value("type").toString();
				double value = readDouble();
				rval.second.insert(type, value);
			}
			else
				readUnknownElement();
		}
	}
	return rval;
}

QHash<QString, QVariant> XkorXmlReader::readOptions()
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

QHash<int, QString> XkorXmlReader::readResults()
{
	QHash<int, QString> rval;
	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("result")) == 0)
			{
				int matchday = attributes().value("matchday").toString().toInt();
				rval[matchday] = readString();
			}
			else
				readUnknownElement();
		}
	}
	return rval;
}

void XkorXmlReader::readRPList()
{
	delete m_rpList;
	m_rpList = new XkorRPList;

	while(!atEnd())
	{
		readNext();
		if(isEndElement())
			break;
		if(isStartElement())
		{
            if(name().compare(QString::fromUtf8("competitionName")) == 0)
				m_rpList->setCompetitionName(readString());
            else if(name().compare(QString::fromUtf8("maxBonus")) == 0)
				m_rpList->setMaxBonus(readDouble());
            else if(name().compare(QString::fromUtf8("minBonus")) == 0)
				m_rpList->setMinBonus(readDouble());
            else if(name().compare(QString::fromUtf8("nation")) == 0)
				m_rpList->addBonus(readNation());
            else if(name().compare(QString::fromUtf8("rpCalculationType")) == 0)
				m_rpList->setRPCalculationType(readString());
            else if(name().compare(QString::fromUtf8("rpEffect")) == 0)
				m_rpList->setRPEffect(readDouble());
            else if(name().compare(QString::fromUtf8("rpOptions")) == 0)
				m_rpList->setRPOptions(readOptions());
            else if(name().compare(QString::fromUtf8("useTeams")) == 0)
				m_rpList->setUseTeams(readString() != "false");
			else
				readUnknownElement();
		}
	}
}

XkorAthlete XkorXmlReader::readSignup()
{
	XkorAthlete ath;
	ath.id = QUuid(attributes().value("id").toString());
	ath.name = attributes().value("name").toString();
	ath.nation = attributes().value("nation").toString();
	ath.skill = attributes().value("skill").toString().toDouble();
	ath.properties = readOptions();

	readNext();
	return ath;
}

QString XkorXmlReader::readString()
{
	return readElementText();
}

void XkorXmlReader::readUnknownElement()
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
