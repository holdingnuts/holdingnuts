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

// cards size
const qreal Seat::sx_card = 107;
const qreal Seat::sy_card = 140;
// mini cards size
const qreal Seat::sx_mini_card = 45;
const qreal Seat::sy_mini_card = 60;

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
	switch ((int)action)
	{
		case Player::None:
				m_pCurrentActionImg = &SeatImages::Instance().imgActNone;
				m_strAmount.clear();
			break;
		case Player::Check:
				m_pCurrentActionImg = &SeatImages::Instance().imgActCheck;
				
				if (amount > 0.0)
					m_strAmount.setNum(amount, 'f', 2);
				else
					m_strAmount.clear();
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
				m_strAmount.clear();
			break;
		case Player::Muck:
				m_pCurrentActionImg = &SeatImages::Instance().imgActMuck;
				m_strAmount.clear();
			break;
		case Player::Allin:
				m_pCurrentActionImg = &SeatImages::Instance().imgActAllin;
				m_strAmount.setNum(amount, 'f', 2);
			break;
	}
}

void Seat::setCurrent(bool cur, int sec_timeout)
{
	m_bCurrent = cur;
/*	
	if (m_bCurrent && sec_timeout)
	{
		m_timeLine.setDuration(sec_timeout * 1000);
		m_timeLine.setFrameRange(0, SeatImages::Instance().imgTimeout.width());
		m_timeLine.start();
	}
	else
		m_timeLine.stop();
*/		
}

void Seat::setSitout(bool sitout)
{
	m_bSitout = sitout;
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
	qreal x = 0;
	qreal y = 0;
	qreal width = SeatImages::Instance().imgBack.width() + m_pCurrentActionImg->width();
	qreal height = SeatImages::Instance().imgBack.height();

	if (m_bMySeat)
		height += sy_card;

	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3
	switch (m_nID)
	{
		case 1: case 2:
				x = -sx_mini_card;
			break;
		case 3: case 4: case 5:
				y = -sy_mini_card;
			break;
		case 8: case 9: case 0:
				height += sy_mini_card;
			break;
		case 6: case 7:
				width += sx_mini_card;
			break;
	}

	QRectF rc(x, y, width, height);
	QTransform m = this->transform();
	
	return m.mapRect(rc);
}

QRectF Seat::boundingRectSeat() const
{
	QRectF rc(
		0,
		0,
		SeatImages::Instance().imgBack.width() + m_pCurrentActionImg->width(),
		SeatImages::Instance().imgBack.height());

	QTransform m = this->transform();
	
	return m.mapRect(rc);
}

void Seat::paint(
	QPainter *painter,
	const QStyleOptionGraphicsItem *option,
	QWidget *widget)
{
	// seat is not occupied, don't paint anything
	if (!m_bValid)
		return;
	
	
	// image card backside
	static QImage imgCardBackside("gfx/deck/default/back.png");
	
	this->setZValue(8);
		
	const qreal seat_width = SeatImages::Instance().imgBack.width();
	const qreal seat_height = SeatImages::Instance().imgBack.height();

	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	
	const QImage *imgBack;
	if (m_bSitout)
		imgBack = &(SeatImages::Instance().imgBackSitout);
	else if (m_bCurrent)
		imgBack = &(SeatImages::Instance().imgBackCurrent);
	else
		imgBack = &(SeatImages::Instance().imgBack);
	
	painter->drawImage(
			QRectF(
			0,
			0,
			seat_width + m_pCurrentActionImg->width(),
			seat_height),
		*imgBack);
/*	
	if (m_bCurrent)
	{
		painter->drawImage(
			QRectF(
				0,
				SeatImages::Instance().imgBackCurrent.height(),
				SeatImages::Instance().imgTimeout.width(),
				SeatImages::Instance().imgTimeout.height()),
			SeatImages::Instance().imgTimeout);
		painter->fillRect(
			QRectF(
				0,
				SeatImages::Instance().imgBackCurrent.height(),
				m_timeLine.currentFrame(),
				SeatImages::Instance().imgTimeout.height()),
			QBrush(Qt::black));
	}
*/	
	// action
	if (m_pCurrentActionImg)
	{
		painter->drawImage(
			QRectF(
				seat_width,
				0,
				m_pCurrentActionImg->width(),
				m_pCurrentActionImg->height()),
			*m_pCurrentActionImg);
	}

	// TODO: font
	static const QFont normal_font("Arial", 18,  QFont::Normal);
	static const QFont bold_font("Arial", 18,  QFont::Bold);
	
	static const QFontMetrics fm_bold(bold_font);
	
	painter->setFont(bold_font);
	painter->drawText(
		QRectF(
			10,
			15,
			seat_width - 10,
			fm_bold.height()),
		Qt::AlignLeft,
		m_strName);
	painter->setFont(normal_font);
	painter->drawText(
		QRectF(
			10,
			seat_height * 0.5,
			seat_width - 10,
			seat_height - 30),
		Qt::AlignLeft,
		"$" + m_strStake);
	
	// TODO: find right textposition
	qreal tx_pos = 0;
	qreal ty_pos = 0;
			
	calcBetTextPos(tx_pos, ty_pos, fm_bold.width(m_strAmount));
	
	painter->setFont(bold_font);
	painter->drawText(
		QRectF(
			tx_pos,
			ty_pos,
			fm_bold.width(m_strAmount),
			fm_bold.height()),
		Qt::AlignLeft,
		m_strAmount);

	// big-cards
	if (m_bBigCards)
	{
		painter->drawImage(
			QRectF(
				seat_width + m_pCurrentActionImg->width() - (sx_card * 1.5),
				-sy_card,
				sx_card,
				sy_card),
			m_FirstCard);
		painter->drawImage(
			QRectF(
				seat_width + m_pCurrentActionImg->width() - sx_card,
				-sy_card,
				sx_card,
				sy_card),
			m_SecondCard);
	}
	
	// small cards
	if (m_bSmallCards)
	{
		qreal sx_pos = 0;
		qreal sy_pos = 0;
		
		calcSCardPos(sx_pos, sy_pos);
		
		painter->drawImage(
			QRectF(
				sx_pos,
				sy_pos,
				sx_mini_card,
				sy_mini_card),
			imgCardBackside);
		painter->drawImage(
			QRectF(
				sx_pos + sx_mini_card * 0.4,
				sy_pos,
				sx_mini_card,
				sy_mini_card),
			imgCardBackside);
	}
}

void Seat::calcSCardPos(qreal& x, qreal& y) const
{
	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3

	switch (m_nID)
	{
		case 1: case 2:
				x = -sx_mini_card * 1.5;
				y = 0;
			break;
		case 3: case 4: case 5:
				x = 0;
				y = -(SeatImages::Instance().imgBack.height() + 5);
			break;
		case 8: case 9: case 0:
				x = 0;
				y = SeatImages::Instance().imgBack.height() + 5;
			break;
		case 6: case 7:
				x = SeatImages::Instance().imgBack.width();
				y = 0;
			break;
	}
}

void Seat::calcBetTextPos(qreal& x, qreal& y, int txt_width) const
{
	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3

	switch (m_nID)
	{
		case 1: case 2:
				x = -(txt_width + 10);
				y = SeatImages::Instance().imgBack.height() - 30;
			break;
		case 3: case 4: case 5:
				x = sx_mini_card * 1.7;
				y = -(SeatImages::Instance().imgBack.height() + 10);
			break;
		case 8: case 9: case 0:
				x = sx_mini_card * 1.7;
				y = SeatImages::Instance().imgBack.height() + 10;
			break;
		case 6: case 7:
				x = SeatImages::Instance().imgBack.width();
				y = 0;
			break;
	}
}
