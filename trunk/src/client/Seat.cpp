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

// cards size
const int Seat::sx_card = 110;
const int Seat::sy_card = 143;
// mini cards size
const int Seat::sx_mini_card = 45;
const int Seat::sy_mini_card = 60;

Seat::Seat(unsigned int id, QWidget *parent)
:	m_nID(id),
	m_bValid(false),
	m_bMySeat(false),
	m_bCurrent(false)
{
	m_pCurrentActionImg = &SeatImages::Instance().imgActNone;
}

void Seat::setValid(bool valid)
{
	m_bValid = valid;
}

void Seat::setName(const QString& name)
{
	m_strName = name;
}

void Seat::setStake(qreal amount)
{
	m_strStake.setNum(amount, 'f', 2);
}

void Seat::setAction(Player::PlayerAction action, qreal amount)
{
	switch (action)
	{
		case Player::None:
				m_pCurrentActionImg = &SeatImages::Instance().imgActNone;
				m_strAmount.clear();
			break;
		case Player::Check:
				m_pCurrentActionImg = &SeatImages::Instance().imgActCheck;
				m_strAmount.setNum(amount, 'f', 2);
			break;
		case Player::Fold:
				m_pCurrentActionImg = &SeatImages::Instance().imgActFold;
			break;
		case Player::Call:
				m_pCurrentActionImg = &SeatImages::Instance().imgActCall;
				m_strAmount.setNum(amount, 'f', 2);
			break;
		case Player::Bet:
				m_pCurrentActionImg = &SeatImages::Instance().imgActBet;
				m_strAmount.setNum(amount, 'f', 2);
			break;
		case Player::Raise:
				m_pCurrentActionImg = &SeatImages::Instance().imgActRaise;
				m_strAmount.setNum(amount, 'f', 2);
			break;
		case Player::Show:
				m_pCurrentActionImg = &SeatImages::Instance().imgActShow;
				m_strAmount.setNum(amount, 'f', 2);
			break;
		case Player::Muck:
				m_pCurrentActionImg = &SeatImages::Instance().imgActMuck;
				m_strAmount.setNum(amount, 'f', 2);
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
	m_FirstCard.load(QString("gfx/deck/default/%1.png").arg(c1));
	m_SecondCard.load(QString("gfx/deck/default/%1.png").arg(c2));
}

QRectF Seat::boundingRect() const
{
	qreal cards;
	
	if (m_bMySeat)
		cards = -sx_card * 1.5;
	else
		cards = -sx_mini_card * 1.5;
	
	return QRectF(
		cards,
		0,
		SeatImages::Instance().imgBack.width() + m_pCurrentActionImg->width(),
		SeatImages::Instance().imgBack.height());
}

void Seat::paint(
	QPainter *painter,
	const QStyleOptionGraphicsItem *option,
	QWidget *widget)
{
	// image card backside
	static QImage imgCardBackside("gfx/deck/default/back.png");
	
	this->setZValue(8);
		
	const int seat_width = SeatImages::Instance().imgBack.width();
	const int seat_height = SeatImages::Instance().imgBack.height();

	if (m_bCurrent)
	{
		painter->drawImage(
			QRect(
				0,
				0,
				seat_width,
				seat_height),
			SeatImages::Instance().imgBackCurrent);
//		painter->drawImage(
//			QRect(
//				wpos.x(),
//				wpos.y() + SeatImages::Instance().imgBackCurrent.height(),
//				SeatImages::Instance().imgTimeout.width(),
//				SeatImages::Instance().imgTimeout.height()),
//			SeatImages::Instance().imgTimeout);
	}
	else
	{
		painter->drawImage(
			QRect(
				0,
				0,
				seat_width,
				seat_height),
			SeatImages::Instance().imgBack);
	}

	// action
	if (m_pCurrentActionImg)
	{
		painter->drawImage(
			QRect(
				seat_width,
				0,
				m_pCurrentActionImg->width(),
				m_pCurrentActionImg->height()),
			*m_pCurrentActionImg);
	}

	// TODO: differ right and left seats
	
	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3
	
	// TODO: font
	painter->setFont(QFont("Arial", 18,  QFont::Bold));
	painter->drawText(
		QRect(
			10,
			0,
			seat_width - 10,
			seat_height),
		Qt::AlignVCenter | Qt::AlignCenter,
		m_strName + "\n" + m_strStake);

	if (!m_bValid)
		return;
		
	painter->drawText(
		QRect(
			-30,
			seat_height,
			50,
			100),
		Qt::AlignRight,
		m_strAmount);

	// cards
	if (m_bMySeat)
	{
		painter->drawImage(
			QRect(
				seat_width + m_pCurrentActionImg->width() - static_cast<int>(sx_card * 1.5),
				-sy_card,
				sx_card,
				sy_card),
			m_FirstCard);
		painter->drawImage(
			QRect(
				seat_width + m_pCurrentActionImg->width() - sx_card,
				-sy_card,
				sx_card,
				sy_card),
			m_SecondCard);
	}
	else // draw small cards
	{
		painter->drawImage(
			QRect(
				static_cast<int>(-sx_mini_card * 1.5),
				0,
				sx_mini_card,
				sy_mini_card),
			imgCardBackside);
		painter->drawImage(
			QRect(
				-sx_mini_card,
				0,
				sx_mini_card,
				sy_mini_card),
			imgCardBackside);
	}
}
