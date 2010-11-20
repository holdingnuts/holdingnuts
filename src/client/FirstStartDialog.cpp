/*
 * Copyright 2008-2010, Dominik Geyer
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

#include "Config.h"

#include "FirstStartDialog.hpp"


FirstStartDialog::FirstStartDialog(ConfigParser &cp, QWidget *parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
	  cfg(&cp)
{
	setWindowTitle("HoldingNuts - " + tr("First start"));
	setWindowIcon(QIcon(":/res/hn_logo.png"));
	setFixedWidth(360);
	
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(actionOk()));
	
	
	QLabel *lblLogo = new QLabel(this);
	QImage image(":res/hn_logo.png");
	lblLogo->setPixmap(QPixmap::fromImage(image));
	
	
	QLabel *lblProjectname = new QLabel("<qt><b><font size=+3>HoldingNuts</font></b></qt>", this);
	
	
	QLabel *lblWelcome = new QLabel("<qt>" +
			tr("This is the first time you started HoldingNuts. "
				"Please confirm the below basic settings.") +
			"</qt>", this);
	lblWelcome->setWordWrap(true);
	

	editPlayerName = new QLineEdit(QString::fromStdString(cfg->get("player_name")), this);
	editPlayerName->setMaxLength(20);
	
	QFormLayout *formPlayerinfo = new QFormLayout;
	formPlayerinfo->addRow(tr("Player name"), editPlayerName);
	

	checkUUID = new QCheckBox(tr("Use unique identifier"));
	checkUUID->setCheckState(Qt::Checked);
	QLabel *lblUUIDInfotext = new QLabel(tr("By using a unique identifier game servers can recognize you "
			"which allows rejoining games after a disconnect and participate in player rankings."), this);
	lblUUIDInfotext->setWordWrap("true");

	QVBoxLayout *uuidLayout = new QVBoxLayout;
	uuidLayout->setSpacing(5);
	uuidLayout->addWidget(checkUUID);
	uuidLayout->addWidget(lblUUIDInfotext);


	QVBoxLayout *contentLayout = new QVBoxLayout;
	contentLayout->addWidget(lblWelcome);
	contentLayout->addLayout(formPlayerinfo);
	contentLayout->addLayout(uuidLayout);
	
	QHBoxLayout *topLayout = new QHBoxLayout;
	topLayout->addWidget(lblLogo, 30, Qt::AlignHCenter | Qt::AlignVCenter);
	topLayout->addWidget(lblProjectname, 70, Qt::AlignLeft | Qt::AlignVCenter);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->setSpacing(20);
	mainLayout->addLayout(topLayout, 30);
	mainLayout->addLayout(contentLayout);
	mainLayout->addWidget(buttonBox, 0, Qt::AlignBottom);
	setLayout(mainLayout);
}

void FirstStartDialog::actionOk()
{
	cfg->set("player_name", editPlayerName->text().toStdString());

	if (checkUUID->checkState() != Qt::Checked)
	{
		// unset UUID
		cfg->set("uuid", "");
	}

	accept();
}
