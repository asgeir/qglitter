// Copyright (c) 2012 AlterEgo Studios
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "QGlitterAppcast.h"

static const char * const kSparkleNamespace = "http://www.andymatuschak.org/xml-namespaces/sparkle";

QGlitterAppcast::QGlitterAppcast()
{
}

QList<QGlitterAppcastItem> QGlitterAppcast::items() const
{
	return m_items;
}

bool QGlitterAppcast::read(QIODevice *data)
{
	m_xmlReader.setDevice(data);

	if (m_xmlReader.readNextStartElement()) {
		if (m_xmlReader.name() == "rss" && m_xmlReader.attributes().value("version") == "2.0") {
			readAppcast();
		} else {
			m_xmlReader.raiseError(QObject::tr("The file is not an RSS version 2.0 file."));
		}
	}

	return !m_xmlReader.error();
}

void QGlitterAppcast::readAppcast()
{
	bool foundChannel = false;
	while (m_xmlReader.readNextStartElement()) {
		if (m_xmlReader.name() == "channel" && foundChannel == false) {
			readChannel();
			foundChannel = true;
		} else {
			m_xmlReader.raiseError(QObject::tr("Unrecognized RSS tag or multiple channels"));
			return;
		}
	}
}

void QGlitterAppcast::readChannel()
{
	while (m_xmlReader.readNextStartElement()) {
		if (m_xmlReader.name() == "title") {
			m_title = m_xmlReader.readElementText();
		} else if (m_xmlReader.name() == "link") {
			m_link = m_xmlReader.readElementText();
		} else if (m_xmlReader.name() == "description") {
			m_description = m_xmlReader.readElementText();
		} else if (m_xmlReader.name() == "language") {
			m_language = m_xmlReader.readElementText();
		} else if (m_xmlReader.name() == "item") {
			readItem();
		} else {
			m_xmlReader.raiseError(QObject::tr("Unrecognized RSS channel tag"));
			return;
		}
	}
}

void QGlitterAppcast::readItem()
{
	QGlitterAppcastItem currentItem;
	QString minimumSystemVersion;

	while (m_xmlReader.readNextStartElement()) {
		if (m_xmlReader.name() == "title") {
			currentItem.setTitle(m_xmlReader.readElementText());
		} else if (m_xmlReader.name() == "pubDate") {
			currentItem.setPublicationDate(QDateTime::fromString(m_xmlReader.readElementText()));
		} else if (m_xmlReader.name() == "releaseNotesLink" && m_xmlReader.namespaceUri() == kSparkleNamespace) {
			QString language = m_xmlReader.attributes().value("xml:lang").toString();
			if (language.size() == 0) {
				if (m_language.size() == 0) {
					language = "en";
				} else {
					language = m_language;
				}
			}

			currentItem.addReleaseNotesUrl(language, m_xmlReader.readElementText());
		} else if (m_xmlReader.name() == "description") {
			QString language = m_xmlReader.attributes().value("xml:lang").toString();
			if (language.size() == 0) {
				if (m_language.size() == 0) {
					language = "en";
				} else {
					language = m_language;
				}
			}

			currentItem.addDescription(language, m_xmlReader.readElementText());
		} else if (m_xmlReader.name() == "minimumSystemVersion" && m_xmlReader.namespaceUri() == kSparkleNamespace) {
			minimumSystemVersion = m_xmlReader.readElementText();
		} else if (m_xmlReader.name() == "enclosure") {
			QXmlStreamAttributes attributes = m_xmlReader.attributes();

			if (attributes.hasAttribute("url") && attributes.hasAttribute("length") && attributes.hasAttribute("type")) {
				currentItem.setMimeType(attributes.value("type").toString());
				currentItem.setMinimumSystemVersion(minimumSystemVersion);
				currentItem.setSize(attributes.value("length").toString().toInt());
				currentItem.setUrl(attributes.value("url").toString());

				currentItem.setDeltaFrom(attributes.value(kSparkleNamespace, "deltaFrom").toString());
				currentItem.setOperatingSystem(attributes.value(kSparkleNamespace, "os").toString());
				currentItem.setShortVersionString(attributes.value(kSparkleNamespace, "shortVersionString").toString());
				currentItem.setSignature(attributes.value(kSparkleNamespace, "dsaSignature").toString());
				currentItem.setVersion(attributes.value(kSparkleNamespace, "version").toString());
			} else {
				m_xmlReader.raiseError(QObject::tr("Invalid RSS enclosure"));
				return;
			}
		} else {
			m_xmlReader.raiseError(QObject::tr("Unrecognized RSS item tag"));
			return;
		}
	}

	m_xmlReader.skipCurrentElement();

	m_items.append(currentItem);
}
