/*
**  This file is part of Vidalia, and is subject to the license terms in the
**  LICENSE file, found in the top level directory of this distribution. If you
**  did not receive the LICENSE file with this file, you may obtain it from the
**  Vidalia source package distributed by the Vidalia Project at
**  http://www.vidalia-project.net/. No part of Vidalia, including this file,
**  may be copied, modified, propagated, or distributed except according to the
**  terms described in the LICENSE file.
*/

/*
** \file HelpBrowser.cpp
** \brief Displays a list of help topics and content
*/

#include "HelpBrowser.h"
#include "Vidalia.h"

#include <QDomDocument>
#include <QDir>

#define LEFT_PANE_INDEX     0
#define NO_STRETCH          0
#define MINIMUM_PANE_SIZE   1

/* Names of elements and attributes in the XML file */
#define ELEMENT_CONTENTS        "Contents"
#define ELEMENT_TOPIC           "Topic"
#define ATTRIBUTE_TOPIC_ID      "id"
#define ATTRIBUTE_TOPIC_HTML    "html"
#define ATTRIBUTE_TOPIC_NAME    "name"
#define ATTRIBUTE_TOPIC_SECTION "section"

/* Define two roles used to store data associated with a topic item */
#define ROLE_TOPIC_ID        Qt::UserRole
#define ROLE_TOPIC_QRC_PATH (Qt::UserRole+1)


/** Constuctor. This will probably do more later */
HelpBrowser::HelpBrowser(QWidget *parent)
 : VidaliaWindow("HelpBrowser", parent)
{
  VidaliaSettings settings;

  /* Invoke Qt Designer generated QObject setup routine */
  ui.setupUi(this);
#if defined(Q_WS_MAC)
  ui.actionHome->setShortcut(QString("Shift+Ctrl+H"));
#endif

  /* Pressing 'Esc' or 'Ctrl+W' will close the window */
  ui.actionClose->setShortcut(QString("Esc"));
  Vidalia::createShortcut("Ctrl+W", this, ui.actionClose, SLOT(trigger()));

  /* Hide Search frame */
  ui.frmFind->setHidden(true);
 
  /* Set the splitter pane sizes so that only the txtBrowser pane expands
   * and set to arbitrary sizes (the minimum sizes will take effect */
  QList<int> sizes;
  sizes.append(MINIMUM_PANE_SIZE); 
  sizes.append(MINIMUM_PANE_SIZE);
  ui.splitter->setSizes(sizes);
  ui.splitter->setStretchFactor(LEFT_PANE_INDEX, NO_STRETCH);

  connect(ui.treeContents,
          SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
          this, SLOT(contentsItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

  connect(ui.treeSearch,
          SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
          this, SLOT(searchItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

  /* Connect the navigation actions to their slots */
  connect(ui.actionHome, SIGNAL(triggered()), ui.txtBrowser, SLOT(home()));
  connect(ui.actionBack, SIGNAL(triggered()), ui.txtBrowser, SLOT(backward()));
  connect(ui.actionForward, SIGNAL(triggered()), ui.txtBrowser, SLOT(forward()));
  connect(ui.txtBrowser, SIGNAL(backwardAvailable(bool)), 
          ui.actionBack, SLOT(setEnabled(bool)));
  connect(ui.txtBrowser, SIGNAL(forwardAvailable(bool)),
          ui.actionForward, SLOT(setEnabled(bool)));
  connect(ui.btnFindNext, SIGNAL(clicked()), this, SLOT(findNext()));
  connect(ui.btnFindPrev, SIGNAL(clicked()), this, SLOT(findPrev()));
  connect(ui.btnSearch, SIGNAL(clicked()), this, SLOT(search()));
  
  /* Load the help topics from XML */
  loadContentsFromXml(":/help/" + language() + "/contents.xml");

  /* Show the first help topic in the tree */
  ui.treeContents->setCurrentItem(ui.treeContents->topLevelItem(0));
  ui.treeContents->setItemExpanded(ui.treeContents->topLevelItem(0), true);
}

/** Called when the user changes the UI translation. */
void
HelpBrowser::retranslateUi()
{
  ui.retranslateUi(this);
  ui.treeContents->clear();
  loadContentsFromXml(":/help/" + language() + "/contents.xml");
  ui.treeContents->setItemExpanded(ui.treeContents->topLevelItem(0), true);
  ui.treeContents->setCurrentItem(ui.treeContents->topLevelItem(0));
  ui.treeContents->setItemExpanded(ui.treeContents->topLevelItem(0), true);
}

/** Returns the language in which help topics should appear, or English
 * ("en") if no translated help files exist for the current GUI language. */
QString
HelpBrowser::language()
{
  QString lang = Vidalia::language();
  if (!QDir(":/help/" + lang).exists())
    lang = "en";
  return lang;
}

/** Load the contents of the help topics tree from the specified XML file. */
void
HelpBrowser::loadContentsFromXml(QString xmlFile)
{
  QString errorString;
  QFile file(xmlFile);
  QDomDocument document;
  
  /* Load the XML contents into the DOM document */
  if (!document.setContent(&file, true, &errorString)) {
    ui.txtBrowser->setPlainText(tr("Error Loading Help Contents: ")+errorString);
    return;
  }
  /* Load the DOM document contents into the tree view */
  if (!loadContents(&document, errorString)) {
    ui.txtBrowser->setPlainText(tr("Error Loading Help Contents: ")+errorString);
    return;
  }
}

/** Load the contents of the help topics tree from the given DOM document. */
bool
HelpBrowser::loadContents(const QDomDocument *document, QString &errorString)
{
  /* Grab the root document element and make sure it's the right one */
  QDomElement root = document->documentElement();
  if (root.tagName() != ELEMENT_CONTENTS) {
    errorString = tr("Supplied XML file is not a valid Contents document.");
    return false;
  }
  _elementList << root;

  /* Create the home item */
  QTreeWidgetItem *home = createTopicTreeItem(root, 0);
  ui.treeContents->addTopLevelItem(home);
  
  /* Process all top-level help topics */
  QDomElement child = root.firstChildElement(ELEMENT_TOPIC);
  while (!child.isNull()) {
    parseHelpTopic(child, home);
    child = child.nextSiblingElement(ELEMENT_TOPIC);
  }
  return true;
}

/** Parse a Topic element and handle all its children recursively. */
void
HelpBrowser::parseHelpTopic(const QDomElement &topicElement, 
                            QTreeWidgetItem *parent)
{
  /* Check that we have a valid help topic */
  if (isValidTopicElement(topicElement)) {
    /* Save this element for later (used for searching) */
    _elementList << topicElement;

    /* Create and populate the new topic item in the tree */
    QTreeWidgetItem *topic = createTopicTreeItem(topicElement, parent);

    /* Process all its child elements */
    QDomElement child = topicElement.firstChildElement(ELEMENT_TOPIC);
    while (!child.isNull()) {
      parseHelpTopic(child, topic);
      child = child.nextSiblingElement(ELEMENT_TOPIC);
    }
  }
}

/** Returns true if the given Topic element has the necessary attributes. */
bool
HelpBrowser::isValidTopicElement(const QDomElement &topicElement)
{
  return (topicElement.hasAttribute(ATTRIBUTE_TOPIC_ID) &&
          topicElement.hasAttribute(ATTRIBUTE_TOPIC_NAME) &&
          topicElement.hasAttribute(ATTRIBUTE_TOPIC_HTML));
}

/** Builds a resource path to an html file associated with the given help
 * topic. If the help topic needs an achor, the anchor will be formatted and
 * appended. */
QString
HelpBrowser::getResourcePath(const QDomElement &topicElement)
{
  QString link = language() + "/" + topicElement.attribute(ATTRIBUTE_TOPIC_HTML);
  if (topicElement.hasAttribute(ATTRIBUTE_TOPIC_SECTION)) {
    link += "#" + topicElement.attribute(ATTRIBUTE_TOPIC_SECTION);
  }
  return link;
}

/** Creates a new element to be inserted into the topic tree. */
QTreeWidgetItem*
HelpBrowser::createTopicTreeItem(const QDomElement &topicElement, 
                                 QTreeWidgetItem *parent)
{
  QTreeWidgetItem *topic = new QTreeWidgetItem(parent);
  QString label = topicElement.attribute(ATTRIBUTE_TOPIC_NAME);

  topic->setText(0, label);
  topic->setToolTip(0, label);
  topic->setData(0, ROLE_TOPIC_ID, topicElement.attribute(ATTRIBUTE_TOPIC_ID));
  topic->setData(0, ROLE_TOPIC_QRC_PATH, getResourcePath(topicElement));

  return topic;
}

/** Called when the user selects a different item in the content topic tree */
void
HelpBrowser::contentsItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *prev)
{
  QList<QTreeWidgetItem *> selected = ui.treeSearch->selectedItems();
  /* Deselect the selection in the search tree */
  if (!selected.isEmpty()) {
    ui.treeSearch->setItemSelected(selected[0], false);
  }
  currentItemChanged(current, prev);
}

/** Called when the user selects a different item in the content topic tree */
void
HelpBrowser::searchItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *prev)
{
  QList<QTreeWidgetItem *> selected = ui.treeContents->selectedItems();
  /* Deselect the selection in the contents tree */
  if (!selected.isEmpty()) {
    ui.treeContents->setItemSelected(selected[0], false);
  }

  /* Change to selected page */
  currentItemChanged(current, prev);

  /* Highlight search phrase */
  QTextCursor found;
  QTextDocument::FindFlags flags = QTextDocument::FindWholeWords;
  found = ui.txtBrowser->document()->find(_lastSearch, 0, flags);
  if (!found.isNull()) {
    ui.txtBrowser->setTextCursor(found);
  }
}

/** Called when the user selects a different item in the tree. */
void
HelpBrowser::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *prev)
{
  Q_UNUSED(prev);
  if (current) {
    ui.txtBrowser->setSource(QUrl(current->data(0, 
                                              ROLE_TOPIC_QRC_PATH).toString()));
  }
  _foundBefore = false;
}

/** Searches for a topic in the topic tree. Returns a pointer to that topics
 * item in the topic tree if it is found, 0 otherwise. */
QTreeWidgetItem*
HelpBrowser::findTopicItem(QTreeWidgetItem *startItem, QString topic)
{
  /* If startItem is null, then we don't know where to start searching. */
  if (!startItem)
    return 0;

  /* Parse the first subtopic in the topic id. */
  QString subtopic = topic.mid(0, topic.indexOf(".")).toLower();

  /* Search through all children of startItem and look for a subtopic match */
  for (int i = 0; i < startItem->childCount(); i++) {
    QTreeWidgetItem *item = startItem->child(i);
    
    if (subtopic == item->data(0, ROLE_TOPIC_ID).toString().toLower()) {
      /* Found a subtopic match, so expand this item */
      ui.treeContents->setItemExpanded(item, true);
      if (!topic.contains(".")) {
        /* Found the exact topic */
        return item;
      }
      /* Search recursively for the next subtopic */
      return findTopicItem(item, topic.mid(topic.indexOf(".")+1));
    }
  }
  return 0;
}

/** Shows the help browser. If a sepcified topic was given, then search for
 * that topic's ID (e.g., "log.basic") and display the appropriate page. */
void
HelpBrowser::showTopic(QString topic)
{
  /* Search for the topic in the contents tree */
  QTreeWidgetItem *item =
    findTopicItem(ui.treeContents->topLevelItem(0), topic);
  QTreeWidgetItem *selected = 0;

  if (item) {
    /* Item was found, so show its location in the hierarchy and select its
     * tree item. */
    if (ui.treeContents->selectedItems().size()) {
      selected = ui.treeContents->selectedItems()[0];
      if (selected)
        ui.treeContents->setItemSelected(selected, false);
    }
    ui.treeContents->setItemExpanded(ui.treeContents->topLevelItem(0), true);
    ui.treeContents->setItemSelected(item, true);
    currentItemChanged(item, selected);
  }
}

/** Called when the user clicks "Find Next". */
void
HelpBrowser::findNext()
{
  find(true);
}

/** Called when the user clicks "Find Previous". */
void
HelpBrowser::findPrev()
{
  find(false);
}

/** Searches the current page for the phrase in the Find box.
 *  Highlights the first instance found in the document
 *  \param forward true search forward if true, backward if false
 **/
void
HelpBrowser::find(bool forward)
{
  /* Don't bother searching if there is no search phrase */
  if (ui.lineFind->text().isEmpty()) {
    return;
  }
  
  QTextDocument::FindFlags flags = 0;
  QTextCursor cursor = ui.txtBrowser->textCursor();
  QString searchPhrase = ui.lineFind->text();
  
  /* Clear status bar */
  this->statusBar()->clearMessage();
  
  /* Set search direction and other flags */
  if (!forward) {
    flags |= QTextDocument::FindBackward;
  }
  if (ui.chkbxMatchCase->isChecked()) {
    flags |= QTextDocument::FindCaseSensitively;
  }
  if (ui.chkbxWholePhrase->isChecked()) {
    flags |= QTextDocument::FindWholeWords;
  }
  
  /* Check if search phrase is the same as the previous */
  if (searchPhrase != _lastFind) {
    _foundBefore = false;
  }
  _lastFind = searchPhrase;
  
  /* Set the cursor to the appropriate start location if necessary */
  if (!cursor.hasSelection()) {
    if (forward) {
      cursor.movePosition(QTextCursor::Start);
    } else {
      cursor.movePosition(QTextCursor::End);
    }
    ui.txtBrowser->setTextCursor(cursor);
  }

  /* Search the page */
  QTextCursor found;
  found = ui.txtBrowser->document()->find(searchPhrase, cursor, flags);
  
  /* If found, move the cursor to the location */
  if (!found.isNull()) {
    ui.txtBrowser->setTextCursor(found);
  /* If not found, display appropriate error message */
  } else {
    if (_foundBefore) {
      if (forward) 
        this->statusBar()->showMessage(tr("Search reached end of document"));
      else 
        this->statusBar()->showMessage(tr("Search reached start of document"));
    } else {
      this->statusBar()->showMessage(tr("Text not found in document"));
    }
  }
  
  /* Even if not found this time, may have been found previously */
  _foundBefore |= !found.isNull();
}
 
/** Searches all help pages for the phrase the Search box.
 *  Fills treeSearch with documents containing matches and sets the
 *  status bar text appropriately.
 */
void
HelpBrowser::search()
{
  /* Clear the list */
  ui.treeSearch->clear();
  
  /* Don't search if invalid document or blank search phrase */
  if (ui.lineSearch->text().isEmpty()) {
    return;
  }
    
  HelpTextBrowser browser;
  QTextCursor found;
  QTextDocument::FindFlags flags = QTextDocument::FindWholeWords;

  _lastSearch = ui.lineSearch->text();

  /* Search through all the pages looking for the phrase */
  for (int i=0; i < _elementList.size(); ++i) {
    /* Load page data into browser */
    browser.setSource(QUrl(getResourcePath(_elementList[i])));
      
    /* Search current document */
    found = browser.document()->find(ui.lineSearch->text(), 0, flags);

    /* If found, add page to tree */
    if (!found.isNull()) {
      ui.treeSearch->addTopLevelItem(createTopicTreeItem(_elementList[i], 0));
    }
  }

  /* Set the status bar text */
  this->statusBar()->showMessage(tr("Found %1 results")
                                .arg(ui.treeSearch->topLevelItemCount()));
}

/** Overrides the default show method */
void
HelpBrowser::showWindow(QString topic)
{
  
  /* Bring the window to the top */
  VidaliaWindow::showWindow();

  /* If a topic was specified, then go ahead and display it. */
  if (!topic.isEmpty()) {
    showTopic(topic);
  }
}

