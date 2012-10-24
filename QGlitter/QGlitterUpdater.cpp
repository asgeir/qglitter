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

#include "QGlitterUpdater.h"
#include "QGlitterAppcast.h"
#include "QGlitterDownloader.h"
#include "QGlitterUpdateAlert.h"
#include "QGlitterUpdateCheckStatus.h"
#include "QGlitterUpdateStatus.h"
#include "QGlitterAutomaticUpdateAlert.h"
#include "Crypto/Crypto.h"
#include "Platform/Platform.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimer>

#include <QDebug>

static const char * const kIsFirstLaunch = "QGlitter/IsFirstLaunch";
static const char * const kAutomaticUpdateCheck = "QGlitter/AutomaticCheck";
static const char * const kAutomaticDownload = "QGlitter/AutomaticDownload";
static const char * const kCheckInterval = "QGlitter/CheckInterval";
static const char * const kIgnoredVersions = "QGlitter/IgnoredVersions";
static const char * const kLastCheckTime = "QGlitter/LastUpdateCheck";

static const qint64 kNeverUpdated = 0;
static const int kOneHour = 60 * 60;
static const int kOneDay = kOneHour * 24;

static const int kBackgroundDownload = 0;
static const int kInteractiveDownload = 1;

QGlitterUpdater::QGlitterUpdater(bool allowVersionSkipping, bool allowDelayInstalUntilQuit, int checkInterval, QObject *parent)
	: QObject(parent)
	, m_applicationIcon(0)
	, m_internalVersion()
	, m_automaticCheck(true)
	, m_automaticDownload(false)
	, m_checkInterval((checkInterval == 0) ? kOneDay : checkInterval)
	, m_defaultLanguage("en")
	, m_feedUrl("")
	, m_allowVersionSkipping(allowVersionSkipping)
	, m_allowDelayInstallUntilQuit(allowDelayInstalUntilQuit)
	, m_isCheckingForUpdates(false)
	, m_isInteractive(false)
	, m_lastUpdateCheck(kNeverUpdated)
	, m_networkAccess(0)
	, m_settings(0)
	, m_timer(0)
	, m_downloader(0)
	, m_pendingUpdate("")
{
	qglitter_cryptoInit();

	m_defaultLanguage = QLocale::system().name();
	m_defaultLanguage.truncate(m_defaultLanguage.lastIndexOf('_'));

	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTimeout()));

	QString settingsDomain = QString("%1.%2").arg(qApp->organizationDomain()).arg(qApp->applicationName().replace(' ', ""));
	m_settings = new QSettings(QSettings::UserScope, "QGlitter", settingsDomain, this);

	m_automaticCheck = m_settings->value(kAutomaticUpdateCheck, true).toBool();
	m_automaticDownload = m_settings->value(kAutomaticDownload, false).toBool();
	m_checkInterval = m_settings->value(kCheckInterval, kOneDay).toInt();
	m_lastUpdateCheck = m_settings->value(kLastCheckTime, kNeverUpdated).value<qint64>();
	m_ignoredVersions = m_settings->value(kIgnoredVersions, QStringList()).toStringList();

	if (!m_settings->value(kIsFirstLaunch, true).toBool()) {
		if (m_lastUpdateCheck == kNeverUpdated) {
			m_timer->start(0);
		} else {
			qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
			qint64 timeUntilNextCheck = m_checkInterval - (currentTime - m_lastUpdateCheck);
			qint64 nextDueTime = (timeUntilNextCheck < 0) ? 0 : timeUntilNextCheck;

			m_timer->start(nextDueTime * 1000);
		}
	}

	m_settings->setValue(kIsFirstLaunch, false);

	m_networkAccess = new QNetworkAccessManager(this);
	connect(m_networkAccess, SIGNAL(finished(QNetworkReply *)), this, SLOT(appcastDownloaded(QNetworkReply *)));

	m_downloader = new QGlitterDownloader(this);
}
QGlitterUpdater::~QGlitterUpdater()
{
	if (m_applicationIcon) {
		delete m_applicationIcon;
	}
}

const QPixmap *QGlitterUpdater::applicationIcon() const
{
	return m_applicationIcon;
}

void QGlitterUpdater::setApplicationIcon(const QPixmap *icon)
{
	if (m_applicationIcon) {
		delete m_applicationIcon;
	}

	m_applicationIcon = new QPixmap(*icon);
}

bool QGlitterUpdater::automaticallyCheckForUpdates() const
{
	return m_automaticCheck;
}

void QGlitterUpdater::setAutomaticallyCheckForUpdates(bool automaticallyCheckForUpdates)
{
	m_automaticCheck = automaticallyCheckForUpdates;
	m_settings->setValue(kAutomaticUpdateCheck, m_automaticCheck);
}

bool QGlitterUpdater::automaticallyDownloadUpdates() const
{
	return m_automaticDownload;
}

void QGlitterUpdater::setAutomaticallyDownloadUpdates(bool automaticallyDownloadUpdates)
{
	m_automaticDownload = automaticallyDownloadUpdates;
	m_settings->setValue(kAutomaticDownload, m_automaticDownload);
}

int QGlitterUpdater::checkInterval() const
{
	return m_checkInterval;
}

void QGlitterUpdater::setCheckInterval(int checkInterval)
{
	if (checkInterval < kOneHour) {
		checkInterval = kOneHour;
	}

	m_checkInterval = checkInterval;
	m_settings->setValue(kCheckInterval, m_checkInterval);

	if (m_timer) {
		m_timer->setInterval(m_checkInterval * 1000);
	}
}

QString QGlitterUpdater::defaultLanguage() const
{
	return m_defaultLanguage;
}

void QGlitterUpdater::setDefaultLanguage(QString language)
{
	m_defaultLanguage = language;
}

QString QGlitterUpdater::feedUrl() const
{
	return m_feedUrl;
}

void QGlitterUpdater::setFeedUrl(QString feedUrl)
{
	m_feedUrl = feedUrl;
}

QByteArray QGlitterUpdater::publicKey() const
{
	return m_publicKey;
}

void QGlitterUpdater::setPublicKey(const QByteArray &publicKey)
{
	m_publicKey = publicKey;
	m_downloader->setPublicKey(publicKey);
}


void QGlitterUpdater::checkForUpdates(const QGlitterAppcast &appcast)
{
	QString currentVersion;
	if (m_internalVersion.size()) {
		currentVersion = m_internalVersion;
	} else {
		currentVersion = qApp->applicationVersion();
	}

	QGlitterAppcastItem currentBestUpdate;
	currentBestUpdate.setVersion(qApp->applicationVersion());

	QList<QGlitterAppcastItem> appcastItems = appcast.items();
	for (int i = 0; i < appcastItems.size(); ++i) {
		if (!m_isInteractive) {
			if (m_ignoredVersions.indexOf(appcastItems[i].version()) >= 0) {
				continue;
			}
		}

		if (appcastItems[i].operatingSystem().length() > 0 && appcastItems[i].operatingSystem().compare(qglitter_osVersion(), Qt::CaseInsensitive) != 0) {
			continue;
		}

		if ((appcastItems[i].version() > currentBestUpdate.version()) ||
			(appcastItems[i].version() == currentBestUpdate.version() && appcastItems[i].deltaFrom() == currentVersion)) {
			currentBestUpdate = appcastItems[i];
		}
	}

	if (currentBestUpdate.version() > currentVersion) {
		emit foundUpdate(currentBestUpdate);

		if (!m_automaticDownload) {

			QGlitterUpdateAlert *updateAlert = new QGlitterUpdateAlert();

			if (m_applicationIcon) {
				updateAlert->setApplicationIcon(m_applicationIcon);
			}
			updateAlert->setAutomaticallyDownloadUpdates(m_automaticDownload);
			updateAlert->setDefaultLanguage(m_defaultLanguage);
			updateAlert->setAppcastItem(currentBestUpdate);
			updateAlert->setAllowSkipping(m_allowVersionSkipping);

			if (updateAlert->exec()) {
				downloadAndInstall(kInteractiveDownload, currentBestUpdate);
			}

			if (updateAlert->skipVersion()) {
				m_ignoredVersions.append(currentBestUpdate.version());
				m_settings->setValue(kIgnoredVersions, m_ignoredVersions);
			}

			if (updateAlert->automaticallyDownloadUpdates()) {
				setAutomaticallyDownloadUpdates(updateAlert->automaticallyDownloadUpdates());
			}

			updateAlert->deleteLater();
		} else {
			downloadAndInstall(kBackgroundDownload, currentBestUpdate);
		}
	} else {
		emit noUpdatesAvailable();
	}
}

void QGlitterUpdater::aboutToQuit()
{
	if (m_pendingUpdate.length() > 0) {
		emit installingUpdate();
		if (qglitter_installUpdate(m_pendingUpdate)) {
			emit finishedInstallingUpdate();
		}
	}
}

void QGlitterUpdater::automaticUpdateDownloaded(int errorCode, QString installerPath)
{
	if (errorCode != QGlitterDownloader::NoError) {
		return;
	}

	// Disconnect in case this update is canceled and the next update is not automatic
	disconnect(m_downloader, SIGNAL(downloadFinished(int, QString)), this, SLOT(automaticUpdateDownloaded(int, QString)));

	// TODO: should probably allow for automatic installing as well
	QGlitterAutomaticUpdateAlert *updateAlert = new QGlitterAutomaticUpdateAlert();

	if (m_applicationIcon) {
		updateAlert->setApplicationIcon(m_applicationIcon);
	}
	updateAlert->setAllowSkipping(m_allowVersionSkipping);
	updateAlert->setAllowDelaying(m_allowDelayInstallUntilQuit);

	updateAlert->exec();
	updateAlert->deleteLater();

	if (updateAlert->result() == QDialog::Rejected) {
		emit updateCanceled();
	} else {
		if (m_allowDelayInstallUntilQuit && updateAlert->delayUntilQuit()) {
			m_pendingUpdate = installerPath;
		} else {
			emit installingUpdate();
			if (qglitter_installUpdate(installerPath)) {
				emit finishedInstallingUpdate();
			}
		}
	}
}

void QGlitterUpdater::downloadAndInstall(int mode, const QGlitterAppcastItem &update)
{
	m_downloader->downloadUpdate(update.url(), update.signature());

	if (mode == kInteractiveDownload) {
		QGlitterUpdateStatus *downloadStatus = new QGlitterUpdateStatus();

		if (m_applicationIcon) {
			downloadStatus->setApplicationIcon(m_applicationIcon);
		}

		connect(m_downloader, SIGNAL(downloadProgress(qint64, qint64)), downloadStatus, SLOT(downloadProgress(qint64, qint64)));
		connect(m_downloader, SIGNAL(downloadFinished(int, QString)), downloadStatus, SLOT(downloadFinished(int, QString)));

		downloadStatus->exec();
		downloadStatus->deleteLater();

		if (downloadStatus->result() == QDialog::Rejected) {
			m_downloader->cancelDownload();
			emit updateCanceled();
		} else if (m_downloader->errorCode() == QGlitterDownloader::NoError) {
			emit installingUpdate();
			if (qglitter_installUpdate(m_downloader->installerFile())) {
				emit finishedInstallingUpdate();
			}
		}
	} else {
		connect(m_downloader, SIGNAL(downloadFinished(int, QString)), this, SLOT(automaticUpdateDownloaded(int, QString)));
	}
}

void QGlitterUpdater::updateCheck()
{
	if (m_isCheckingForUpdates) {
		return;
	}

	m_isInteractive = true;
	m_isCheckingForUpdates = true;
	QNetworkReply *reply = m_networkAccess->get(QNetworkRequest(QUrl(m_feedUrl)));

	QGlitterUpdateCheckStatus *updateCheckStatus = new QGlitterUpdateCheckStatus();
	connect(this, SIGNAL(foundUpdate(const QGlitterAppcastItem &)), updateCheckStatus, SLOT(close()));
	connect(this, SIGNAL(noUpdatesAvailable()), updateCheckStatus, SLOT(noUpdatesAvailable()));
	connect(this, SIGNAL(errorLoadingAppcast()), updateCheckStatus, SLOT(noUpdatesAvailable()));

	updateCheckStatus->setNetworkReply(reply);
	if (m_applicationIcon) {
		updateCheckStatus->setApplicationIcon(m_applicationIcon);
	}

	updateCheckStatus->exec();
	updateCheckStatus->deleteLater();

	m_isInteractive = false;
}

void QGlitterUpdater::backgroundUpdateCheck()
{
	m_timer->singleShot(0, this, SLOT(updateTimeout()));
}

void QGlitterUpdater::appcastDownloaded(QNetworkReply *reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		QGlitterAppcast appcast;

		if (appcast.read(reply)) {
			emit finishedLoadingAppcast(appcast);
			checkForUpdates(appcast);
		} else {
			emit errorLoadingAppcast();
		}
	}

	m_isCheckingForUpdates = false;
	m_lastUpdateCheck = QDateTime::currentMSecsSinceEpoch() / 1000;
	m_timer->setInterval(m_checkInterval * 1000);

	reply->deleteLater();
}

void QGlitterUpdater::updateTimeout()
{
	if (m_isCheckingForUpdates) {
		return;
	}

	m_isCheckingForUpdates = true;
	m_networkAccess->get(QNetworkRequest(QUrl(m_feedUrl)));
}

