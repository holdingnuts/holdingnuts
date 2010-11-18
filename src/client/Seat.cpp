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
#include "ChipStack.hpp"

#include <QPainter>
#include <QFont>

extern ConfigParser config;

// cards size
const qreal Seat::sx_card = 96;
const qreal Seat::sy_card = 138;

// mini cards size
const qreal Seat::sx_mini_card = 41;
const qreal Seat::sy_mini_card = 58;

// In-Seat-Font
QFont Seat::m_ftInSeat;
QFontMetrics Seat::m_fmInSeat(Seat::m_ftInSeat);

Seat::Seat(unsigned int id, QWidget *parent)
:	m_nID(id),
	m_bValid(false),
	m_bMySeat(false),
	m_bCurrent(false),
	m_xPosSmallCards(0),
	m_yPosSmallCards(0)
{
	m_pCurrentActionImg = &SeatImages::Instance().imgActNone;
	m_pChipStack = new ChipStack(this);
	
	this->setZValue(8);
	
	// precalc position
	calcSCardPos(m_xPosSmallCards, m_yPosSmallCards);
}

void Seat::setValid(bool valid)
{
	m_bValid = valid;
	
	if (!valid)
	{
		this->setToolTip(QString());	// unset tooltip
		m_pChipStack->hide();
	}
}

void Seat::setInfo(const QString& name, const QString& location)
{
	m_strName = name;
	
	chopName();
	
	if (m_strName.length() < name.length())
		m_strName.append("...");
	
	QString tooltip(tr("Name: %1").arg(name));
	
	if (!location.isEmpty())
		tooltip.append(QString("\n") + tr("Location: %1").arg(location));
	
	this->setToolTip(tooltip);
}

void Seat::setStake(chips_type amount)
{
	m_strStake.setNum(amount);
}

void Seat::setAction(Player::PlayerAction action, chips_type amount)
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
	
	if (amount > 0)
		m_strAmount.setNum(amount);
	else
		m_strAmount.clear();
		
	m_pChipStack->setAmount(amount);
}

void Seat::setWin(chips_type amount)
{	
	if (amount > 0)
	{
		m_strAmount.setNum(amount);
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
				height += sy_mini_card + 5;
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
	
	static const qreal seat_width = SeatImages::Instance().imgBack.width();
	static const qreal seat_height = SeatImages::Instance().imgBack.height();
	
	const QImage *imgBack;
	if (m_bSitout)
		imgBack = &(SeatImages::Instance().imgBackSitout);
	else if (m_bCurrent)
		imgBack = &(SeatImages::Instance().imgBackCurrent);
	else
		imgBack = &(SeatImages::Instance().imgBack);
	
	painter->save();

	painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

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

	// text name
	QPainterPath pathTxtName;
	pathTxtName.addText(10, m_fmInSeat.height() + 10, m_ftInSeat, m_strName);
	
	painter->fillPath(pathTxtName, Qt::black);
	
	// text stake
	QPainterPath pathTxtStake;
	pathTxtStake.addText(10, 75, m_ftInSeat, m_strStake);
	
	painter->fillPath(pathTxtStake, Qt::black);

	// text amount
	qreal tx_pos = 0;
	qreal ty_pos = 0;
			
	calcBetTextPos(tx_pos, ty_pos, m_fmInSeat.width(m_strAmount));

	painter->setFont(m_ftInSeat);
	painter->drawText(
		QRectF(
			tx_pos,
			ty_pos,
			m_fmInSeat.width(m_strAmount),
			m_fmInSeat.height()),
		Qt::AlignLeft,
		m_strAmount);

	// chip stake
	qreal csx_pos = 0;
	qreal csy_pos = 0;
	
	calcChipStackPos(csx_pos, csy_pos, m_fmInSeat.width(m_strAmount));

	m_pChipStack->setPos(csx_pos, csy_pos);
	m_pChipStack->update();

	// big-cards
	if (m_bBigCards)
	{
		qreal bc1x_pos = 0;
		qreal bc2x_pos = 0;

		if (m_nID >= 3 && m_nID <= 7)
		{
			bc1x_pos = 0;
			bc2x_pos = sx_card * 0.5;
		}
		else // m_nID < 3 || m_nID > 7
		{
			bc1x_pos = seat_width + m_pCurrentActionImg->width() - (sx_card * 1.5);
			bc2x_pos = seat_width + m_pCurrentActionImg->width() - sx_card;
		}
		
		painter->drawPixmap(
			QRectF(
				bc1x_pos,
				-(sy_card + 2),
				sx_card,
				sy_card),
			m_FirstCard,
			QRectF(0 , 0, m_FirstCard.width(), m_FirstCard.height()));
		painter->drawPixmap(
			QRectF(
				bc2x_pos,
				-(sy_card + 2),
				sx_card,
				sy_card),
			m_SecondCard,
			QRectF(0 , 0, m_SecondCard.width(), m_SecondCard.height()));
	}
	
	// small cards
	if (m_bSmallCards)
	{
		painter->drawImage(
			QRectF(
				m_xPosSmallCards,
				m_yPosSmallCards,
				sx_mini_card,
				sy_mini_card),
			imgCardBackside);
		painter->drawImage(
			QRectF(
				m_xPosSmallCards + sx_mini_card * 0.4,
				m_yPosSmallCards,
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
				x = SeatImages::Instance().imgBack.width() - (sx_mini_card * 0.4) - 5;
				y = -(sy_mini_card + 5);
			break;
		case 8: case 9: case 0:
				x = 0;
				y = SeatImages::Instance().imgBack.height() + 5;
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
				x = 175;
				y = -(sy_mini_card + 35);
			break;
		case 8: case 9: case 0:
				x = 0;
				y = SeatImages::Instance().imgBack.height() + sy_mini_card + 10;
			break;
		case 6: case 7:
				x = SeatImages::Instance().imgBack.width() + sx_mini_card + 10;
				y = SeatImages::Instance().imgBack.height() - 30;
			break;
	}
}

void Seat::calcChipStackPos(qreal& x, qreal& y, int txt_width) const
{
	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3

	switch (m_nID)
	{
		case 1: case 2:
				x = -(sx_mini_card * 1.7 + 10); // width from both small cards
				y = -30;
			break;
		case 3: case 4: case 5:
				x = 155;
				y = -(sy_mini_card + 60);
			break;
		case 8: case 9: case 0:
				x = 70;
				y = 158;
			break;
		case 6: case 7:
				x = sx_card * 1.6;
				y = -30;
			break;
	}
}

void Seat::chopName()
{
	while (m_fmInSeat.width(m_strName) > (SeatImages::Instance().imgBack.width() - 
									SeatImages::Instance().imgActNone.width()))
	{
		m_strName.chop(1);
		chopName();
	}
}

void Seat::setInSeatFont(const QFont& font)
{
	m_ftInSeat = font;
	m_fmInSeat = QFontMetrics(m_ftInSeat);
	
	m_ftInSeat.setStyleStrategy(QFont::ForceOutline);
}
