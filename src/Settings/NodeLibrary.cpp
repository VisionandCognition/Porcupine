/*
 * Copyright (C) 2014 Tim van Mourik
*/

#include <iostream>
#include <cstring>
#include <QDomDocument>
#include <QXmlStreamReader>

#include "Argument.hpp"
#include "DataType.hpp"
#include "NodeLibrary.hpp"
#include "NodeSetting.hpp"

NodeLibrary::NodeLibrary() :
    Singleton(),
    m_nodeValidator(0)
{
}

void NodeLibrary::setDataTypeSchema(
        QFile& _typeSchema
        )
{
    _typeSchema.open(QIODevice::ReadOnly);
    QXmlSchema xmlSchema;
    xmlSchema.load(_typeSchema.readAll());
    if (!xmlSchema.isValid())
    {
        std::cerr << "Error: Nodes cold not be loaded.\n";
    }
    m_nodeValidator = new QXmlSchemaValidator(xmlSchema);
}

void NodeLibrary::setNodeSchema(
        QFile& _nodeSchema
        )
{
    _nodeSchema.open(QIODevice::ReadOnly);
    QXmlSchema xmlSchema;
    xmlSchema.load(_nodeSchema.readAll());
    if (!xmlSchema.isValid())
    {
        std::cerr << "Error: Nodes cold not be loaded.\n";
    }
    m_nodeValidator = new QXmlSchemaValidator(xmlSchema);
}

void NodeLibrary::addDataTypes(
        QFile& _schema
        )
{
    QDomDocument document;
    if(!_schema.open(QIODevice::ReadOnly))
    {
        std::cerr << "Error: cannot open file.\n";
        return;
    }

    if(!document.setContent(&_schema))
    {
        std::cerr << "Error: cannot read file.\n";
        _schema.close();
        return;
    }
    _schema.reset();
    QByteArray instanceText = _schema.readAll();
    if(!m_nodeValidator->validate(instanceText))
    {
        std::cerr << "Error: The data type file has an incorrect format.\n";
        _schema.close();
        return;
    }

    QDomElement docElement = document.documentElement();
    QDomNode n = docElement.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement();
        if(e.tagName().compare("typename") == 0)
        {
            QString title;
            title = e.firstChild().nodeValue();
            m_dataTypes[title] = new DataType(title);
            m_typeNames << title;
        }
        n = n.nextSibling();
    }
}

QString NodeLibrary::addNodeSetting(
        QFile& _schema
        )
{
    _schema.open(QIODevice::ReadOnly);
    QByteArray instanceText = _schema.readAll();
    if(m_nodeValidator->validate(instanceText))
    {
        _schema.reset();
        QDomDocument document;
        document.setContent(&_schema);
        _schema.close();

        QDomElement docElem = document.documentElement();
        QString rootTag = docElem.tagName();
        if(std::strcmp(rootTag.toStdString().c_str(), "node") == 0)
        {
//            std::cerr << rootTag.toStdString() << "\n"; //Should print 'node'
            Argument title;
            QVector<Argument> inputNodes;
            QVector<Argument> inOutNodes;
            QVector<Argument> outputNodes;
            QDomNode node = docElem.firstChild();
            while(!node.isNull())
            {
                if(node.isElement())
                {
                    if(node.nodeName().compare("title") == 0)
                    {
                        title.setName(node.attributes().namedItem("name").nodeValue());
                        ///@todo add 'category' atribute
                        // if there is a code block
                        QDomNode code = node.firstChild();
                        if(!code.isNull() && code.nodeName().compare("code") == 0)
                        {
                            QString language;
                            QString argument;
                            QString comment;
                            parseCodeBlock(code, language, argument, comment);
                            title.addCode(language, argument, comment);
                        }
                    }
                    else if(node.nodeName().compare("input") == 0)
                    {
                        Argument codeArgument(node.attributes().namedItem("name").nodeValue());
                        // if there is a code block
                        QDomNode code = node.firstChild();
                        if(!code.isNull() && code.nodeName().compare("code") == 0)
                        {
                            QString language;
                            QString argument;
                            QString comment;
                            parseCodeBlock(code, language, argument, comment);
                            codeArgument.addCode(language, argument, comment);
                        }
                        inputNodes.append(codeArgument);
                    }
                    else if(node.nodeName().compare("input-output") == 0)
                    {
                        Argument codeArgument(node.attributes().namedItem("name").nodeValue());
                        // if there is a code block
                        QDomNode code = node.firstChild();
                        if(!code.isNull() && code.nodeName().compare("code") == 0)
                        {
                            QString language;
                            QString argument;
                            QString comment;
                            parseCodeBlock(code, language, argument, comment);
                            codeArgument.addCode(language, argument, comment);
                        }
                        inOutNodes.append(codeArgument);
                    }
                    else if(node.nodeName().compare("output") == 0)
                    {
                        Argument codeArgument(node.attributes().namedItem("name").nodeValue());
                        // if there is a code block
                        QDomNode code = node.firstChild();
                        if(!code.isNull() && code.nodeName().compare("code") == 0)
                        {
                            QString language;
                            QString argument;
                            QString comment;
                            parseCodeBlock(code, language, argument, comment);
                            codeArgument.addCode(language, argument, comment);
                        }
                        outputNodes.append(codeArgument);
                    }
                    node = node.nextSibling();
                }
            }
//            std::cout << title.toStdString() << std::endl;
            m_nodeSettings[title.getName()] = new NodeSetting(title, inputNodes, inOutNodes, outputNodes);
            m_nodeNames << title.getName();
            return title.getName();
        }
        else
        {
            std::cerr << "This node is invalid. The root is not a node.\n";
            return QString("");
        }
    }
    else
    {
        std::cerr << "This node is invalid. It has an invalid structure.\n";
        return QString("");
    }
}

void NodeLibrary::parseCodeBlock(
        const QDomNode& _code,
        QString& o_language,
        QString& o_argument,
        QString& o_comment
        )
{
    QDomNode codeBlock = _code.firstChild();
    while(!codeBlock.isNull())
    {
        if(codeBlock.nodeName().compare("language") == 0)
        {
            o_language = codeBlock.attributes().namedItem("name").nodeValue();
            QDomNode argumentBlock = codeBlock.firstChild();
            while(!argumentBlock.isNull())
            {
                if(argumentBlock.nodeName().compare("argument") == 0)
                {
                    o_argument = argumentBlock.attributes().namedItem("name").nodeValue();
                }
                else if(argumentBlock.nodeName().compare("comment") == 0)
                {
                    o_comment = argumentBlock.attributes().namedItem("text").nodeValue();
                }
                argumentBlock = argumentBlock.nextSibling();
            }
        }
        codeBlock = codeBlock.nextSibling();
    }
}

const NodeSetting* NodeLibrary::getNodeSetting(
        const QString& _nodeName
        ) /*const*/
{
    return m_nodeSettings[_nodeName];
}

const QStringList& NodeLibrary::getNodeNames(
        ) const
{
    return m_nodeNames;
}

NodeLibrary::~NodeLibrary()
{
    std::map <QString, NodeSetting*>::iterator nodeIterator = m_nodeSettings.begin();
//    std::cerr << "NodeLibrary Destructed\n";
//    std::cout << m_nodeSettings.size() << std::endl;
    while(nodeIterator != m_nodeSettings.end())
    {
//        std::cerr << "Deleting NodeSetting...\n";
        delete (*nodeIterator).second;
        ++nodeIterator;
    }

    std::map <QString, DataType*>::iterator dataIterator = m_dataTypes.begin();
    while(dataIterator != m_dataTypes.end())
    {
        delete (*dataIterator).second;
        ++dataIterator;
    }

    delete m_nodeValidator;
}












