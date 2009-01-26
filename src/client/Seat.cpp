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
 *     Michael Miller <michael.miller@holdingnuts.net>
 */


#include <QPainter>
#include <QFont>

#include "Seat.hpp"
#include "SeatImages.hpp"
#include "Debug.h"

WPicture::WPicture(const char *filename, QWidget *parent) : QLabel(parent)
{
	//setBackgroundRole(QPalette::Base);
	//setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	//sizePolicy().hasHeightForWidth();
//	setScaledContents(true);
	
	loadImage(filename);
}

void WPicture::loadImage(const char *filename)
{
	QImage image(filename);
	setPixmap(QPixmap::fromImage(image));
}

/*
int WPicture::heightForWidth ( int w )
{
	return -1;
}
*/

////////////////////////

Seat::Seat(unsigned int id, QWidget *parent)
:	m_nID(id),
	m_bValid(false),
	m_bMySeat(false),
	m_bCurrent(false)
{
	m_pCurrentActionImg = &SeatImages::Instance().imgActNone;
	

	//setPalette(Qt::gray);
	//setAutoFillBackground(true);
//	setFixedSize(100, 70);
	
//	QImage image("gfx/table/seat.png");
//	setPixmap(QPixmap::fromImage(image));
//	setScaledContents(true);
	
//	lblCaption = new QLabel("Seat");
//	lblCaption->setAlignment(Qt::AlignCenter);
//	lblStake = new QLabel("0.00");
//	lblStake->setAlignment(Qt::AlignCenter);
//	lblAction = new QLabel("Action");
//	lblAction->setAlignment(Qt::AlignCenter);
	
	///////
	int sx = 45;
	int sy = 60;
	card1 = new WPicture("gfx/deck/default/back.png", parent);
	card1->setFixedSize(sx, sy);
	card2 = new WPicture("gfx/deck/default/back.png", parent);
	card2->setFixedSize(sx, sy);
	
	// miniature cards
	sx = 17;
	sy = 25;
	scard1 = new WPicture("gfx/deck/default/back.png", parent);
	scard1->setFixedSize(sx, sy);
	scard2 = new WPicture("gfx/deck/default/back.png", parent);
	scard2->setFixedSize(sx, sy);
	
	//QHBoxLayout *lCards = new QHBoxLayout();
	//lCards->addWidget(card1);
	//lCards->addWidget(card2);
	
//	QVBoxLayout *layout = new QVBoxLayout();
	// layout->addWidget(lblCaption);
	//layout->addLayout(lCards);
//	layout->addWidget(lblStake);
//	layout->addWidget(lblAction);
//	setLayout(layout);
}

void Seat::setValid(bool valid)
{
	m_bValid = valid;
//	lblCaption->setVisible(valid);
//	lblStake->setVisible(valid);
//	lblAction->setVisible(valid);
}

void Seat::setName(const QString& name)
{
//	lblCaption->setText(name);
	m_strName = name;
}

void Seat::setStake(float amount)
{
	m_strAmount.setNum(amount, 'f', 2);
}

void Seat::setAction(Player::PlayerAction action, float amount)
{
//	if (action == Player::Fold)
//		lblAction->setText("Folded");
//	else
//		lblAction->setText(m_strAmount);

	switch (action)
	{
		case Player::Check:
				m_pCurrentActionImg = &SeatImages::Instance().imgActCheck;
			break;
		case Player::Fold:
				m_pCurrentActionImg = &SeatImages::Instance().imgActFold;
			break;
		case Player::Call:
				m_pCurrentActionImg = &SeatImages::Instance().imgActCall;
			break;
		case Player::Bet:
				m_pCurrentActionImg = &SeatImages::Instance().imgActBet;
			break;
		case Player::Raise:
				m_pCurrentActionImg = &SeatImages::Instance().imgActRaise;
			break;
		case Player::Show:
				m_pCurrentActionImg = &SeatImages::Instance().imgActShow;
			break;
		case Player::Muck:
				m_pCurrentActionImg = &SeatImages::Instance().imgActMuck;
			break;
		default:
			break;
	}
}

void Seat::setCurrent(bool cur)
{
	m_bCurrent = cur;
}

void Seat::setMySeat(bool bMyseat)
{
	m_bMySeat = bMyseat;
}

void Seat::setCards(const char *c1, const char *c2)
{
	char filename[1024];
	
	snprintf(filename, sizeof(filename), "gfx/deck/default/%s.png", c1);
	card1->loadImage(filename);
	
	snprintf(filename, sizeof(filename), "gfx/deck/default/%s.png", c2);
	card2->loadImage(filename);
	
	m_FirstCard.load(QString("gfx/deck/default/%1.png").arg(c1));
	m_SecondCard.load(QString("gfx/deck/default/%1.png").arg(c2));
}

void Seat::move(int x, int y)
{
	this->setPos(x, y);
}

int Seat::width() const
{
	return 100;
}

int Seat::height() const
{
	return 70;
}

QRectF Seat::boundingRect() const
{
	// TODO: right size
	return QRectF(0, 0, width(), height());
}

void Seat::paint(
	QPainter *painter,
	const QStyleOptionGraphicsItem *option,
	QWidget *widget)
{
	// cards size
	static const int sx_card = 110;
	static const int sy_card = 143;
	// mini cards size
	static const int sx_mini_card = 45;
	static const int sy_mini_card = 60;
	// image card backside
	static QImage imgCardBackside("gfx/deck/default/back.png");
	
	this->setZValue(9);

	const QPoint wpos(
		static_cast<int>(this->scenePos().x()),
		static_cast<int>(this->scenePos().y()));

	if (m_bCurrent)
	{
		painter->drawImage(
			QRect(
				wpos.x(),
				wpos.y(),
				SeatImages::Instance().imgBackCurrent.width(),
				SeatImages::Instance().imgBackCurrent.height()),
			SeatImages::Instance().imgBackCurrent);
		painter->drawImage(
			QRect(
				wpos.x(),
				wpos.y() + SeatImages::Instance().imgBackCurrent.height(),
				SeatImages::Instance().imgTimeout.width(),
				SeatImages::Instance().imgTimeout.height()),
			SeatImages::Instance().imgTimeout);
	}
	else
	{
		painter->drawImage(
			QRect(
				wpos.x(),
				wpos.y(),
				SeatImages::Instance().imgBack.width(),
				SeatImages::Instance().imgBack.height()),
			SeatImages::Instance().imgBack);
	}

	if (!m_bValid)
		return;

	// action
	if (m_pCurrentActionImg)
	{
		painter->drawImage(
			QRect(
				wpos.x() + SeatImages::Instance().imgBack.width(),
				wpos.y(),
				m_pCurrentActionImg->width(),
				m_pCurrentActionImg->height()),
			*m_pCurrentActionImg);
	}

	// TODO: font
	painter->setFont(QFont("Arial", 18));
	painter->drawText(
		QRect(
			wpos.x() + 10,
			wpos.y(),
			SeatImages::Instance().imgBack.width(),
			SeatImages::Instance().imgBack.height()),
		Qt::AlignVCenter,
		m_strName + "\n" + m_strAmount);

	// cards
	if (m_bMySeat)
	{
		painter->drawImage(
			QRect(
				wpos.x() - sx_card,
				wpos.y(),
				sx_card,
				sy_card),
			m_FirstCard);
		painter->drawImage(
			QRect(
				wpos.x() - sx_card / 2,
				wpos.y(),
				sx_card,
				sy_card),
			m_SecondCard);
	}
	else // draw small cards
	{
		painter->drawImage(
			QRect(
				wpos.x() - sx_mini_card,
				wpos.y(),
				sx_mini_card,
				sy_mini_card),
			imgCardBackside);
		painter->drawImage(
			QRect(
				wpos.x() - sx_mini_card / 2,
				wpos.y(),
				sx_mini_card,
				sy_mini_card),
			imgCardBackside);
	}
}
