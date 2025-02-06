#ifndef XMLINDEX_H
#define XMLINDEX_H

#include <map>
#include <QString>

class XkorXmlIndex
{
	private:
		std::map<QString, QString> index;
		void insert(QString filename);
	public:
        void traverse(QString dir);
        QString lookup(QString name);
		std::map<QString, QString> getAllFiles();
};

#endif
