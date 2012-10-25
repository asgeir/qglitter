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

#include "QGlitterUpdateAlert.h"
#include "ui_QGlitterUpdateAlert.h"

#include <QPixmap>

const char * const kBoldText = "<html><body><p><span style=\" font-weight:600;\">%1</span></p></body></html>";

QGlitterUpdateAlert::QGlitterUpdateAlert(QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f | Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint)
	, m_ui(new Ui_QGlitterUpdateAlert)
	, m_defaultLanguage("en")
	, m_skipVersion(false)
	, m_automaticDownloads(false)
{
	if (!m_ui) {
		return;
	}

	m_ui->setupUi(this);

	connect(m_ui->enableAutomaticDownload, SIGNAL(toggled(bool)), this, SLOT(toggleAutomaticDownloads()));
	connect(m_ui->installUpdateButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect(m_ui->remindMeLaterButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
	connect(m_ui->skipVersionButton, SIGNAL(clicked(bool)), this, SLOT(toggleSkipVersion()));

	m_ui->headerLabel->setText(QString(kBoldText).arg(QObject::tr("A new version of %1 is available!").arg(qApp->applicationName())));
}

QGlitterUpdateAlert::~QGlitterUpdateAlert()
{
	if (m_ui) {
		delete m_ui;
	}
}

void QGlitterUpdateAlert::setDefaultLanguage(const QString &defaultLanguage)
{
	m_defaultLanguage = defaultLanguage;
}

void QGlitterUpdateAlert::setAllowSkipping(bool allowSkipping)
{
	if (!m_ui) {
		return;
	}

	m_ui->skipVersionButton->setVisible(allowSkipping);
}

void QGlitterUpdateAlert::setAppcastItem(const QGlitterAppcastItem &appcastItem)
{
	m_appcastItem = appcastItem;

	if (!m_ui) {
		return;
	}

	QString message = QObject::tr("%1 %2 is now available &mdash; you have %3. Would you like to download it now?");
	message = message.arg(qApp->applicationName());
	message = message.arg(appcastItem.version());
	message = message.arg(qApp->applicationVersion());
	m_ui->messageLabel->setText(QString("<html><body><p>%1</p></body></html>").arg(message));

	QMap<QString, QString> descriptions = appcastItem.descriptions();
	QMap<QString, QString> releaseNotesUrls = appcastItem.releaseNotesUrls();

	if (descriptions.find(m_defaultLanguage) != descriptions.end()) {
		m_ui->releaseNotes->setHtml(descriptions[m_defaultLanguage]);
	} else if (releaseNotesUrls.find(m_defaultLanguage) != releaseNotesUrls.end()) {
		m_ui->releaseNotes->setUrl(releaseNotesUrls[m_defaultLanguage]);
	} else if (descriptions.size() > 0) {
		QMapIterator<QString, QString> i(descriptions);
		if (i.hasNext()) {
			m_ui->releaseNotes->setHtml(i.next().value());
		}
	} else {
		QMapIterator<QString, QString> i(releaseNotesUrls);
		if (i.hasNext()) {
			m_ui->releaseNotes->setUrl(i.next().value());
		}
	}
}

void QGlitterUpdateAlert::setApplicationIcon(const QPixmap *applicationIcon)
{
	if (!m_ui) {
		return;
	}

	m_ui->iconLabel->setPixmap(applicationIcon->scaled(m_ui->iconLabel->size()));
}

bool QGlitterUpdateAlert::automaticallyDownloadUpdates() const
{
	return m_automaticDownloads;
}

void QGlitterUpdateAlert::setAutomaticallyDownloadUpdates(bool automaticallyDownloadUpdates)
{
	m_automaticDownloads = automaticallyDownloadUpdates;

	if (m_ui) {
		m_ui->enableAutomaticDownload->setChecked(automaticallyDownloadUpdates);
	}
}

bool QGlitterUpdateAlert::skipVersion()
{
	return m_skipVersion;
}

void QGlitterUpdateAlert::toggleSkipVersion()
{
	m_skipVersion = !m_skipVersion;
	reject();
}

void QGlitterUpdateAlert::toggleAutomaticDownloads()
{
	m_automaticDownloads = !m_automaticDownloads;
}