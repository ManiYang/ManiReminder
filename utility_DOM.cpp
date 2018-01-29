//20171204

#include "utility_DOM.h"

void nsDomUtil::add_child_element_w_text(QDomDocument &doc, QDomElement &parent,
                                         const QString &tag_name, const QString &text,
                                         const bool as_CDATA_section)
//Add
//  <tag_name> text </tag_name>
//under 'parent'.
{
    QDomElement element = doc.createElement(tag_name);
    parent.appendChild(element);

    if(! as_CDATA_section)
    {
        QDomText text_node = doc.createTextNode(text);
        element.appendChild(text_node);
    }
    else
    {
        QDomCDATASection CDATA_node = doc.createCDATASection(text);
        element.appendChild(CDATA_node);
    }
}

QDomElement nsDomUtil::add_child_element(QDomDocument &doc, QDomElement &parent,
                                         const QString &tag_name)
//Add and child element with tag-name tag_name under 'parent'. Returns the child element
//added.
{
    QDomElement child = doc.createElement(tag_name);
    parent.appendChild(child);
    return child;
}

bool nsDomUtil::is_an_element_containing_text(QDomNode &node, QString *tag_name, QString *text)
//Return whether 'node' is of the form
//  <tag_name> text </tag_name>
//where the text can also include CDATA sections and/or comments, and can be empty.
//If so, *text will be assigned the text (if 'text' is not NULL) and *tag_name will be
//assigned the tag name (if 'tag_name' is not NULL).
//For example, the node
//  <tag>
//    <!--comment-->
//    some text &amp; <![CDATA[<b>a CDATA section</b>]]> in &lt;tag&gt;
//  </tag>
//will be recognized as containing the text
//  "some text & <b>a CDATA section</b> in <tag>"
{
    if(! node.isElement())
        return false;

    //
    QString str;
    QDomNodeList child_nodes = node.childNodes();
    for(int i=0; i<child_nodes.count(); i++)
    {
        QDomNode node = child_nodes.at(i);
        if(node.isText())
            str += node.toText().data();
        else if(node.isCDATASection())
            str += node.toCDATASection().data();
        else if(node.isComment())
            ;
        else //(is not text, CDATA section, or comment)
            return false;
    }

    //
    if(tag_name != Q_NULLPTR)
        *tag_name = node.toElement().tagName();
    if(text != Q_NULLPTR)
        *text = str;
    return true;
}

QStringList nsDomUtil::get_texts_of_child_elements(QDomElement &parent,
                                                   const QString &tag_name)
//Get the texts in the direct child elements of the form
//  <tag_name> text </tag_name>
//under 'parent'.
//The text can include CDATA sections and/or comments, and can be empty.
{
    QStringList texts;

    QDomNodeList child_nodes = parent.childNodes();
    for(int i=0; i<child_nodes.count(); i++)
    {
        QDomNode child_node = child_nodes.at(i);
        QString tname, text;
        bool ch = is_an_element_containing_text(child_node, &tname, &text);
        if(!ch)
            continue;
        if(tname != tag_name)
            continue;

        texts << text;
    }

    return texts;
}

QString nsDomUtil::get_text_of_first_child_element(QDomElement &parent,
                                                   const QString &tag_name)
//Get the text in the first of direct child elements of the form
//  <tag_name> text </tag_name>
//under 'parent'.
//The text can include CDATA sections and/or comments, and can be empty.
//Return null string (not empty string) if not found.
{
    QDomNodeList child_nodes = parent.childNodes();
    for(int i=0; i<child_nodes.count(); i++)
    {
        QDomNode child_node = child_nodes.at(i);
        QString tname, text;
        bool ch = is_an_element_containing_text(child_node, &tname, &text);
        if(ch)
        {
            if(tname == tag_name)
                return text;
        }
    }

    return QString();
}

QDomElement nsDomUtil::get_ith_child_element(QDomElement &parent,
                                             const int i_, const QString &tag_name)
//Get the ith (counted from 0) direct child element with specified tag name under 'parent'.
//Return null element if not found.
{
    QDomElement element = parent.firstChildElement(tag_name);
    if(element.isNull())
        return element;

    for(int j=0; j<i_; j++)
    {
        element = element.nextSiblingElement(tag_name);
        if(element.isNull())
            return element;
    }
    return element;
}

int nsDomUtil::get_count_of_child_elements(QDomElement &parent, const QString &tag_name)
//Get the number of child element with the specified tag name directly under `parent`.
{
     QDomElement element = parent.firstChildElement(tag_name);
     if(element.isNull())
         return 0;

     for(int i=1; true; i++)
     {
         element = element.nextSiblingElement(tag_name);
         if(element.isNull())
             return i;
     }

     return -1; //should not get here
}
