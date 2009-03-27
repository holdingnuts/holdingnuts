/*
 * Copyright 2008, 2009, Dominik Geyer
 *
 * This file is part of HoldingNuts.
 *
 * HoldingNuts is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoldingNuts is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoldingNuts.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


#include <QtGui>
#include <QDesktopServices>
#include <QUrl>

#include "Config.h"

#include "AboutDialog.hpp"


AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	setWindowTitle(tr("About"));
	setWindowIcon(QIcon(":/res/hn_logo.png"));
	setFixedSize(350, 230);
	
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	
	
	QLabel *lblLogo = new QLabel(this);
	QImage image(":res/hn_logo.png");
	lblLogo->setPixmap(QPixmap::fromImage(image));
	
	
	QLabel *lblProjectname = new QLabel("<qt><b><font size=+3>HoldingNuts</font></b></qt>", this);
	
	
	QLabel *lblVersion = new QLabel("<qt><b>" +
					tr("Version %1.%2.%3")
						.arg(VERSION_MAJOR)
						.arg(VERSION_MINOR)
						.arg(VERSION_REVISION) + "</b> <i>(" +
						QString("SVN %1").arg(VERSIONSTR_SVN) + 
					")</i></qt>",
					 this);
	
	
	QLabel *lblLicense = new QLabel(tr("Licensed under the GPLv3"));
	
	QLabel *lblCopyright = new QLabel("Copyright 2008, 2009, Dominik Geyer &\nHoldingNuts team", this);
	
	QLabel *lblWebsite = new QLabel(this);
	lblWebsite->setText("<qt><a href=\"http://www.holdingnuts.net/\">http://www.holdingnuts.net/</a></qt>");
	connect(lblWebsite, SIGNAL(linkActivated(const QString&)), this, SLOT(actionHyperlink(const QString&)));
	
	
	QLabel *lblMail = new QLabel(this);
	lblMail->setText("<qt><a href=\"mailto:contact@holdingnuts.net\">contact@holdingnuts.net</a></qt>");
	connect(lblMail, SIGNAL(linkActivated(const QString&)), this, SLOT(actionHyperlink(const QString&)));
	
	
	QVBoxLayout *contentLayout = new QVBoxLayout;
	contentLayout->addWidget(lblProjectname);
	contentLayout->addWidget(lblVersion);
	contentLayout->addWidget(lblLicense);
	contentLayout->addWidget(lblCopyright);
	contentLayout->addWidget(lblWebsite);
	contentLayout->addWidget(lblMail);
	
	QHBoxLayout *baseLayout = new QHBoxLayout;
	baseLayout->addWidget(lblLogo, 30, Qt::AlignHCenter | Qt::AlignTop);
	baseLayout->addLayout(contentLayout, 70);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(baseLayout);
	mainLayout->addWidget(buttonBox, 0, Qt::AlignBottom);
	setLayout(mainLayout);
}

void AboutDialog::actionHyperlink(const QString &link)
{
	QDesktopServices::openUrl(QUrl(link));
}
