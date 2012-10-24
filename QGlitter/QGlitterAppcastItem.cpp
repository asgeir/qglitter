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

#include "QGlitterAppcastItem.h"

QGlitterAppcastItem::QGlitterAppcastItem()
	: m_deltaFrom("")
	, m_descriptions()
	, m_mimeType("")
	, m_minimumSystemVersion("")
	, m_operatingSystem("")
	, m_publicationDate()
	, m_releaseNotesUrls()
	, m_shortVersionString("")
	, m_signature("")
	, m_size(0)
	, m_title("")
	, m_url("")
	, m_version("")
{
}

QString QGlitterAppcastItem::deltaFrom() const
{
	return m_deltaFrom;
}

void QGlitterAppcastItem::setDeltaFrom(QString deltaFrom)
{
	m_deltaFrom = deltaFrom;
}

QMap<QString, QString> QGlitterAppcastItem::descriptions() const
{
	return m_descriptions;
}

void QGlitterAppcastItem::addDescription(QString language, QString description)
{
	m_descriptions[language] = description;
}

QString QGlitterAppcastItem::mimeType() const
{
	return m_mimeType;
}

void QGlitterAppcastItem::setMimeType(QString mimeType)
{
	m_mimeType = mimeType;
}

QString QGlitterAppcastItem::minimumSystemVersion() const
{
	return m_minimumSystemVersion;
}

void QGlitterAppcastItem::setMinimumSystemVersion(QString minimumSystemVersion)
{
	m_minimumSystemVersion = minimumSystemVersion;
}

QString QGlitterAppcastItem::operatingSystem() const
{
	return m_operatingSystem;
}

void QGlitterAppcastItem::setOperatingSystem(QString operatingSystem)
{
	m_operatingSystem = operatingSystem;
}

QDateTime QGlitterAppcastItem::publicationDate() const
{
	return m_publicationDate;
}

void QGlitterAppcastItem::setPublicationDate(QDateTime publicationDate)
{
	m_publicationDate = publicationDate;
}

QMap<QString, QString> QGlitterAppcastItem::releaseNotesUrls() const
{
	return m_releaseNotesUrls;
}

void QGlitterAppcastItem::addReleaseNotesUrl(QString language, QString releaseNotesUrl)
{
	m_releaseNotesUrls[language] = releaseNotesUrl;
}

QString QGlitterAppcastItem::shortVersionString() const
{
	if (m_shortVersionString.size() == 0) {
		return m_version;
	}

	return m_shortVersionString;
}

void QGlitterAppcastItem::setShortVersionString(QString shortVersionString)
{
	m_shortVersionString = shortVersionString;
}

QString QGlitterAppcastItem::signature() const
{
	return m_signature;
}

void QGlitterAppcastItem::setSignature(QString signature)
{
	m_signature = signature;
}

int QGlitterAppcastItem::size() const
{
	return m_size;
}

void QGlitterAppcastItem::setSize(int size)
{
	m_size = size;
}

QString QGlitterAppcastItem::title() const
{
	return m_title;
}

void QGlitterAppcastItem::setTitle(QString title)
{
	m_title = title;
}

QString QGlitterAppcastItem::url() const
{
	return m_url;
}

void QGlitterAppcastItem::setUrl(QString url)
{
	m_url = url;
}

QString QGlitterAppcastItem::version() const
{
	return m_version;
}

void QGlitterAppcastItem::setVersion(QString version)
{
	m_version = version;
}
