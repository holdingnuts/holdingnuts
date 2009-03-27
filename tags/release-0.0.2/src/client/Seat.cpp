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


#include "Seat.hpp"
#include "SeatImages.hpp"
#include "Debug.h"
#include "ConfigParser.hpp"

#include <QPainter>
#include <QFont>

extern ConfigParser config;

// cards size
const qreal Seat::sx_card = 96;
const qreal Seat::sy_card = 138;

// mini cards size
const qreal Seat::sx_mini_card = 41;
const qreal Seat::sy_mini_card = 58;

Seat::Seat(unsigned int id, QWidget *parent)
:	m_nID(id),
	m_bValid(false),
	m_bMySeat(false),
	m_bCurrent(false)
{
	m_pCurrentActionImg = &SeatImages::Instance().imgActNone;
	
	this->setZValue(8);
}

void Seat::setValid(bool valid)
{
	m_bValid = valid;
}

void Seat::setInfo(const QString& name, const QString& location)
{
	m_strName = name;
	this->setToolTip(tr("Location: %1").arg(location));
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
			break;
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
		case Player::Allin:
				m_pCurrentActionImg = &SeatImages::Instance().imgActAllin;
			break;
	}
	
	if (amount > 0.0)
		m_strAmount.setNum(amount, 'f', 2);
	else
		m_strAmount.clear();
}

void Seat::setWin(qreal amount)
{	
	if (amount > 0.0)
	{
		m_strAmount.setNum(amount, 'f', 2);
		m_pCurrentActionImg = &SeatImages::Instance().imgStatusWin;
	}
	else
	{
		m_strAmount.clear();
		m_pCurrentActionImg = &SeatImages::Instance().imgActNone;
	}
}

void Seat::setCurrent(bool cur)
{
	m_bCurrent = cur;
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
	m_FirstCard.load(QString("gfx/deck/%1/%2.png")
		.arg(QString::fromStdString(config.get("ui_card_deck")))
		.arg(c1));
	
	m_SecondCard.load(QString("gfx/deck/%1/%2.png")
		.arg(QString::fromStdString(config.get("ui_card_deck")))
		.arg(c2));
}

QRectF Seat::boundingRect() const
{
	Q_ASSERT_X(m_pCurrentActionImg, Q_FUNC_INFO, "bad action image pointer");

	qreal x = 0;
	qreal y = 0;
	qreal width = SeatImages::Instance().imgBack.width() + m_pCurrentActionImg->width();
	qreal height = SeatImages::Instance().imgBack.height();

	// bigcards
	y = -(sy_card + 2);
	height += sy_card + 2;

	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3
	 
	// smallcards
	switch (m_nID)
	{
		case 1: case 2:
				x = -(sx_mini_card * 1.4 + 5);
				width += sx_mini_card * 1.4 + 5;
			break;
		case 3: case 4: case 5:
			/* smallcards size includes already in bigcards size */
			break;
		case 8: case 9: case 0:
				height += sy_mini_card + 15;
			break;
		case 6: case 7:
				width += sx_mini_card * 1.5;
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
	static QImage imgCardBackside(QString("gfx/deck/%1/back.png")
		.arg(QString::fromStdString(config.get("ui_card_deck"))));
	
	const qreal seat_width = SeatImages::Instance().imgBack.width();
	const qreal seat_height = SeatImages::Instance().imgBack.height();
	
	const QImage *imgBack;
	if (m_bSitout)
		imgBack = &(SeatImages::Instance().imgBackSitout);
	else if (m_bCurrent)
		imgBack = &(SeatImages::Instance().imgBackCurrent);
	else
		imgBack = &(SeatImages::Instance().imgBack);
	
	
	painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	
	
	painter->save();
	
	painter->drawImage(
			QRectF(
			0,
			0,
			seat_width + m_pCurrentActionImg->width(),
			seat_height),
		*imgBack);
	
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
	static QFont normal_font("Arial", 18,  QFont::Normal);
	static QFont bold_font("Arial", 18,  QFont::Bold);
	
	normal_font.setStyleStrategy(QFont::ForceOutline);
	bold_font.setStyleStrategy(QFont::ForceOutline);
	
	static const QFontMetrics fm_bold(bold_font);

	QPainterPath pathTxtName;
	pathTxtName.addText(10, fm_bold.height() + 10, bold_font, m_strName);
	
	painter->fillPath(pathTxtName, Qt::black);
	
	QPainterPath pathTxtStake;
	pathTxtStake.addText(10, 75, normal_font, m_strStake);
	
	painter->fillPath(pathTxtStake, Qt::black);

	qreal tx_pos = 0;
	qreal ty_pos = 0;
			
	calcBetTextPos(tx_pos, ty_pos, fm_bold.width(m_strAmount));
	
	// TODO: QPainterPath test
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
		painter->drawPixmap(
			QRectF(
				seat_width + m_pCurrentActionImg->width() - (sx_card * 1.5),
				-(sy_card + 2),
				sx_card,
				sy_card),
			m_FirstCard,
			QRectF(0 , 0, m_FirstCard.width(), m_FirstCard.height()));
		painter->drawPixmap(
			QRectF(
				seat_width + m_pCurrentActionImg->width() - sx_card,
				-(sy_card + 2),
				sx_card,
				sy_card),
			m_SecondCard,
			QRectF(0 , 0, m_SecondCard.width(), m_SecondCard.height()));
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
	
#ifdef DEBUG	
	if (config.getBool("dbg_bbox"))
	{
		painter->setPen(Qt::blue);
		painter->drawRect(this->boundingRect());
	}
#endif
	
	painter->restore();
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
				y = -(sy_mini_card + 5);
			break;
		case 8: case 9: case 0:
				x = 0;
				y = SeatImages::Instance().imgBack.height() + 15;
			break;
		case 6: case 7:
				x = SeatImages::Instance().imgBack.width() + sx_mini_card + 5;
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
				x = 0;
				y = -(SeatImages::Instance().imgBack.height() + 10);
			break;
		case 8: case 9: case 0:
				x = sx_mini_card * 1.7;
				y = SeatImages::Instance().imgBack.height() + 20;
			break;
		case 6: case 7:
				x = SeatImages::Instance().imgBack.width() + sx_mini_card + 10;
				y = SeatImages::Instance().imgBack.height() - 30;
			break;
	}
}
