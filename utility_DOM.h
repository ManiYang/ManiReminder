//20171204

#ifndef UTILITIE_DOM_H
#define UTILITIE_DOM_H

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QStringList>

namespace nsDomUtil
{
void add_child_element_w_text(QDomDocument &doc, QDomElement &parent,
                              const QString &tag_name, const QString &text,
                              const bool as_CDATA_section = false);
//Add
//  <tag_name> text </tag_name>
//under 'parent'.

QDomElement add_child_element(QDomDocument &doc, QDomElement &parent,
                              const QString &tag_name);
//Add a child element with tag name tag_name under 'parent'. Returns the child element
//added.

bool is_an_element_containing_text(QDomNode &node,
                                   QString *tag_name = Q_NULLPTR, QString *text = Q_NULLPTR);
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

QStringList get_texts_of_child_elements(QDomElement &parent, const QString &tag_name);
//Get the texts in the direct child elements of the form
//  <tag_name> text </tag_name>
//under 'parent'.
//The text can include CDATA sections and/or comments, and can be empty.

QString get_text_of_first_child_element(QDomElement &parent, const QString &tag_name);
//Get the text in the first of direct child elements of the form
//  <tag_name> text </tag_name>
//under 'parent'.
//The text can include CDATA sections and/or comments, and can be empty.
//Return null string (not empty string) if not found.

int get_count_of_child_elements(QDomElement &parent, const QString &tag_name);
//Get the number of child element with the specified tag name directly under `parent`.

QDomElement get_ith_child_element(QDomElement &parent, const int i, const QString &tag_name);
//Get the ith (counted from 0) child element with the specified tag name under 'parent'.
//Return null element if not found.

}

#endif // UTILITIE_DOM_H
