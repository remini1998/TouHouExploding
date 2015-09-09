#include <string>
#include "CSceneBattle.h"
#include "CSceneMenu.h"
#include "Effects.h"
#include "CCard.h"

USING_NS_CC;

bool gbTest = true;

std::string WStrToUTF8(const std::wstring& src)
{
	std::string dest;
	for (size_t i = 0; i < src.size(); i++){
		wchar_t w = src[i];
		if (w <= 0x7f)
			dest.push_back((char)w);
		else if (w <= 0x7ff)
		{
			dest.push_back(0xc0 | ((w >> 6) & 0x1f));
			dest.push_back(0x80 | (w & 0x3f));
		}
		else if (w <= 0xffff)
		{
			dest.push_back(0xe0 | ((w >> 12) & 0x0f));
			dest.push_back(0x80 | ((w >> 6) & 0x3f));
			dest.push_back(0x80 | (w & 0x3f));
		}
		else if (sizeof(wchar_t) > 2 && w <= 0x10ffff)
		{
			//dest.push_back(0xf0 | ((w >> 18) & 0x07)); // wchar_t 4-bytes situation  
			dest.push_back(0x80 | ((w >> 12) & 0x3f));
			dest.push_back(0x80 | ((w >> 6) & 0x3f));
			dest.push_back(0x80 | (w & 0x3f));
		}
		else
			dest.push_back('?');
	}
	return dest;
}

bool CSceneBattle::init()
{
	//layerBackGround
	nodeBackGround = CSLoader::createNode("BattleScene.csb");
	layerBattleScene = Layer::create();
	layerBattleScene->addChild(nodeBackGround);
	addChild(layerBattleScene);
	currentScene = sceneBattle;

	buttonAbandonedCards = nodeBackGround->getChildByName<ui::Button *>("Button_AbandonedCards");
	buttonAbandonedCards->addClickEventListener(CC_CALLBACK_0(CSceneBattle::OnButtonAbandonedCards, this));
	buttonSwitch = nodeBackGround->getChildByName<ui::Button *>("Button_Switch");
	buttonSwitch->addClickEventListener(CC_CALLBACK_0(CSceneBattle::OnButtonSwitch, this));
	buttonGiveUp = nodeBackGround->getChildByName<ui::Button *>("Button_GiveUp");
	buttonGiveUp->addClickEventListener(CC_CALLBACK_0(CSceneBattle::OnButtonGiveUp, this));
	buttonEndTurn = nodeBackGround->getChildByName<ui::Button *>("Button_EndTurn");
	buttonEndTurn->addClickEventListener(CC_CALLBACK_0(CSceneBattle::OnButtonEndTurn, this));

	bHandCards = true;
	//chessBoard
	chessBoard = CLayerChessBoard::create();
	layerBattleScene->addChild(chessBoard);
	//layerShade
	layerShade = Layer::create();
	LayerColor *layerColor = LayerColor::create();
	layerColor->setColor(Color3B(0, 0, 0));
	layerColor->setOpacity(120);
	layerShade->setVisible(false);
	layerShade->addChild(layerColor);
	addChild(layerShade);
	//nodePauseMenu
	nodePauseMenu = CSLoader::createNode("PauseMenu.csb");
	layerBattleScene->addChild(nodePauseMenu);
	nodePauseMenu->setVisible(false);
	addChild(nodePauseMenu);

	auto buttonReturnMainMenu = nodePauseMenu->getChildByName<ui::Button *>("Button_ReturnMainMenu");
	buttonReturnMainMenu->addClickEventListener(CC_CALLBACK_0(CSceneBattle::OnButtonReturnMainMenu, this));
	auto buttonContinue = nodePauseMenu->getChildByName<ui::Button *>("Button_ContinueGame");
	buttonContinue->addClickEventListener(CC_CALLBACK_0(CSceneBattle::OnButtonContinue, this));
	//nodeAbandonedCards
	nodeAbandonedCards = CSLoader::createNode("AbandonedCards.csb");
	layerBattleScene->addChild(nodeAbandonedCards);
	
	scrollViewAbandonedCards = nodeAbandonedCards->getChildByName<ui::ScrollView *>("ScrollView_AbandonedCards");
	nodeAbandonedCards->setVisible(false);
	//panelCards
	panelCards = nodeBackGround->getChildByName<ui::Layout *>("Panel_HandCards");
	nodeHandCards = Node::create();
	panelCards->addChild(nodeHandCards);
	//nodeSummonPool
	nodeSummonPool = Node::create();
	nodeSummonPool->setVisible(false);
	panelCards->addChild(nodeSummonPool);
	//cardDescribe
	textCardDescribe = nodeBackGround->getChildByName<ui::Text *>("Text_CardDescribe");
	textRoundCountdown = nodeBackGround->getChildByName<ui::Text *>("Text_RoundCountdown");
	textTotalRound = nodeBackGround->getChildByName<ui::Text *>("Text_TotalRound");
	//esc键
	auto listenerKeyBoard = EventListenerKeyboard::create();
	listenerKeyBoard->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event *event)
	{
		//按下了ESC
		if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
		{
			switch (currentScene)
			{
			case sceneBattle:
				nodePauseMenu->setVisible(true);
				buttonAbandonedCards->setTouchEnabled(false);
				buttonSwitch->setTouchEnabled(false);
				buttonEndTurn->setTouchEnabled(false);
				buttonGiveUp->setTouchEnabled(false);
				textCardDescribe->setString("");
				layerShade->setVisible(true);
				currentScene = scenePauseMenu;
				break;
			case scenePauseMenu:
				OnButtonContinue();
				break;
			case sceneAbandonedCards:
				AbandonedCardsRetuen();
				break;
			}
			log("%s", "esc");
		}
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listenerKeyBoard, this);
	//鼠标事件
	auto listenerMouse = EventListenerMouse::create();
	//鼠标移动
	listenerMouse->onMouseMove = [this](Event *event)
	{
		EventMouse *eventMouse = (EventMouse *)event;
		if (sceneBattle == currentScene)
		{
			Point point = eventMouse->getLocationInView();
			//显示选择的格子
			chessBoard->MouseMove(point.x, point.y);
			//显示鼠标停留的卡片的描述
			Node *node;
			if (true == bHandCards)
				node = nodeHandCards;
			else
				node = nodeSummonPool;
			auto vec = node->getChildren();
			for (auto it : vec)
			{
				Sprite *card = (Sprite *)it;
				Rect cardRect = GetCardViewRect(card, panelCards->getPosition());
				if (cardRect.containsPoint(point))
				{
					std::wstring wstr = L"还没开始写描述";
					textCardDescribe->setString(WStrToUTF8(wstr));
					break;
				}
				else{
					textCardDescribe->setString("");
				}
			}
		}
		log("%s %f %f", "MouseMove", eventMouse->getCursorX(), eventMouse->getCursorY());
	};
	//鼠标点击
	Point *prevPoint = new Point(0, 0);
	EScene *prevScene = new EScene(sceneBattle);
	listenerMouse->onMouseDown = [this, prevPoint, prevScene](Event *event)
	{
		EventMouse *eventMouse = (EventMouse *)event;
		*prevPoint = eventMouse->getLocationInView();
		*prevScene = currentScene;
		log("%s %f %f", "MouseDown", eventMouse->getCursorX(), eventMouse->getCursorY());
	};
	listenerMouse->onMouseUp = [this, prevPoint, prevScene](Event *event)
	{
		if (*prevScene != currentScene)
			return;
		EventMouse *eventMouse = (EventMouse *)event;
		if (sceneBattle == currentScene)
		{
			if (true == bHandCards)
			{
				//删除点击的Card
				auto vec = nodeHandCards->getChildren();
				for (auto it : vec)
				{
					CCard *card = (CCard *)it;
					Rect cardRect = GetCardViewRect(card, panelCards->getPosition());
					if (cardRect.containsPoint(*prevPoint) &&
						cardRect.containsPoint(eventMouse->getLocationInView()))
					{
						nodeHandCards->removeChild(it);
						it->removeFromParent();
						_RedrawCards(nodeHandCards);
						textCardDescribe->setString("");
						event->stopPropagation();
						break;
					}
				}
			}
		}
		else if (sceneAbandonedCards == currentScene){
			if (gbTest)
			{
				AddAbandonedCards(0);
			}
			Rect rectScrollView;
			rectScrollView.size = scrollViewAbandonedCards->getCustomSize();
			rectScrollView.origin = scrollViewAbandonedCards->getPosition();

			if (!rectScrollView.containsPoint(*prevPoint) &&
				!rectScrollView.containsPoint(eventMouse->getLocationInView()))
			{
				AbandonedCardsRetuen();
			}
		}
		prevPoint->set(0.0, 0.0);

		log("%s %f %f", "MouseUp", eventMouse->getCursorX(), eventMouse->getCursorY());
	};
	//鼠标滚轮
	listenerMouse->onMouseScroll = [this](Event *event)
	{
		EventMouse *eventMouse = (EventMouse *)event;
		switch (currentScene)
		{
		case sceneAbandonedCards:
		{
			auto layout = scrollViewAbandonedCards->getInnerContainer();
			float t1 = abs(layout->getPositionX());
			float t2 = layout->getContentSize().width;
			//log("%s %f %f", "scroll", t1,t2);

			float currentPercent = (t2 == 800) ? 0 : t1 * 100 / (t2 - 800);
			float fAddPercent = (layout->getContentSize().width == 800) ? 0 : eventMouse->getScrollY() * 200 * 100 / (layout->getContentSize().width - 800);
			float fLackPercent = 0;
			currentPercent += fAddPercent;
			if (currentPercent < 0)
			{
				fLackPercent = -currentPercent;
				currentPercent = 0;
			}
			else if (currentPercent > 100){
				fLackPercent = currentPercent - 100;
				currentPercent = 100;
			}
			if(fLackPercent != abs(fAddPercent))
				scrollViewAbandonedCards->scrollToPercentHorizontal(currentPercent, (((fAddPercent == 0.f) ? 1 : 1 - fLackPercent / abs(fAddPercent))) * 0.2F, false);
		}
			break;
		}
	};

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listenerMouse, panelCards);
	//
	//countdown
	//
	return true;
}

Scene* CSceneBattle::createScene()
{
	auto scene = CSceneBattle::create();
	auto layer = Layer::create();
	scene->addChild(layer);
	return scene;
}

void CSceneBattle::_RedrawCards(Node *node)
{
	auto vec = node->getChildren();
	int cardsCount = node->getChildrenCount(),
		middle = panelCardWidth / 2,
		n = 0;
	float begX = middle - ((cardsCount - 1)*cardWidth*cardScale / 2);
	for (auto it : vec)
	{
		Sprite *card = (Sprite *)it;
		card->setPosition(begX + n*cardWidth*cardScale, 0);
		++n;
	}
}

char *GetCardFileName(char *fileName, int cardID)
{
	//返回 Card_ID.png 格式的图片名
	strcpy(fileName, "Card_");
	char strID[10];
	_itoa(cardID, strID, 10);
	strcat(fileName, strID);
	strcat(fileName, ".png");
	return fileName;
}

void CSceneBattle::AddHandCards(int cardID)
{
	//init
	CCard *card = CCard::create(cardID);
	card->setAnchorPoint(Point(0.5, 0));
	card->setScale(cardScale);

	nodeHandCards->addChild(card);
	_RedrawCards(nodeHandCards);
}

void CSceneBattle::AddSummonPool(int cardID)
{
	CCard *card = CCard::create(cardID);
	card->setAnchorPoint(Point(0.5, 0));
	card->setScale(cardScale);

	nodeSummonPool->addChild(card);
	_RedrawCards(nodeSummonPool);
}

void CSceneBattle::AddAbandonedCards(int cardID)
{
	int iCardsCount = scrollViewAbandonedCards->getChildrenCount();
	Size sizeScrollView = scrollViewAbandonedCards->getCustomSize();
	auto layout = scrollViewAbandonedCards->getInnerContainer();
	layout->setContentSize(Size(MAX((cardWidth * cardScale + 10)*(iCardsCount + 1) + 80,sizeScrollView.width), 
		sizeScrollView.height));
	CCard *card = CCard::create(cardID);
	card->ShowCard(Point((cardWidth * cardScale + 10)*(iCardsCount+0.5) + 40, sizeScrollView.height / 2), 
		cardScale, 
		scrollViewAbandonedCards);
}

void CSceneBattle::OnButtonReturnMainMenu()
{
	if (currentScene != scenePauseMenu)
		return;
	auto scene = CSceneMenu::create();//Scene();
	Director::getInstance()->replaceScene(scene);
}

void CSceneBattle::OnButtonContinue()
{
	if (currentScene != scenePauseMenu)
		return;
	nodePauseMenu->setVisible(false);

	buttonAbandonedCards->setTouchEnabled(true);
	buttonSwitch->setTouchEnabled(true);
	buttonEndTurn->setTouchEnabled(true);
	buttonGiveUp->setTouchEnabled(true);

	layerShade->setVisible(false);
	currentScene = sceneBattle;
}

void CSceneBattle::OnButtonAbandonedCards()
{
	if (currentScene != sceneBattle)
		return;
	buttonAbandonedCards->setEnabled(false);
	scrollViewAbandonedCards->jumpToLeft();
	nodeAbandonedCards->setAnchorPoint(Vec2(0.5F, 0.5F));
	nodeAbandonedCards->setVisible(true);
	//动画
	nodeAbandonedCards->setPosition(buttonAbandonedCards->getPosition());
	nodeAbandonedCards->setScale(0);
	auto spawn = Spawn::create(ScaleTo::create(0.3F, 1), MoveTo::create(0.3F, Point(1440 / 2, 900 / 2)), NULL);
	nodeAbandonedCards->runAction(Sequence::create(spawn,
		CallFunc::create([this](){buttonAbandonedCards->setEnabled(true); }),
		NULL));

	currentScene = sceneAbandonedCards;
}

void CSceneBattle::AbandonedCardsRetuen()
{
	//由弃牌堆返回
	if (!buttonAbandonedCards->isEnabled())
		return;
	buttonAbandonedCards->setEnabled(false);
	auto spawn = Spawn::create(ScaleTo::create(0.3F, 0), MoveTo::create(0.3F, buttonAbandonedCards->getPosition()), NULL);
	nodeAbandonedCards->runAction(Sequence::create(spawn,
		CallFunc::create([this](){nodeAbandonedCards->setVisible(false); buttonAbandonedCards->setEnabled(true);}),
		NULL));
	
	currentScene = sceneBattle;
}

void CSceneBattle::OnButtonSwitch()
{
	auto button = nodeBackGround->getChildByName<ui::Button *>("Button_Switch");
	if (!button->isEnabled())
		return;
	button->setEnabled(false);
	if (bHandCards)
	{
		nodeHandCards->setVisible(false);
		nodeSummonPool->setVisible(true);
		bHandCards = false;
		//看起来像旋转了按钮的动画
		button->runAction(Sequence::create(ScaleTo::create(0.15F, 0, 1),
			CallFunc::create([button](){button->setTitleText(WStrToUTF8(L"手牌")); }),
			ScaleTo::create(0.15F, 1, 1),
			CallFunc::create([button](){button->setEnabled(true); }),
			NULL));
	}
	else{
		nodeSummonPool->setVisible(false);
		nodeHandCards->setVisible(true);
		bHandCards = true;
		//看起来像旋转了按钮的动画
		button->runAction(Sequence::create(ScaleTo::create(0.15F, 0, 1),
			CallFunc::create([button](){button->setTitleText(WStrToUTF8(L"召唤池")); }),
			ScaleTo::create(0.15F, 1, 1),
			CallFunc::create([button](){button->setEnabled(true); }),
			NULL));
	}
}

void DelHandCards(int cardID)
{
	log("DeleteHandCard:%d", cardID);
}

void CSceneBattle::RoundCountdown(float t)
{
	schedule(schedule_selector(CSceneBattle::UpdateRoundCountdown), 1.F);
	roundRestTime = t;
	timeCount = t;

	std::wstring wstr = L"回合倒计时：", wtime;
	std::wstringstream wss(wtime);
	wss << timeCount;
	wss >> wtime;
	textRoundCountdown->setString(WStrToUTF8(wstr + wtime));

}

void CSceneBattle::AddTotalRound()
{
	++totalRound;
	std::wstring wstr = L"总回合数：", wround;
	std::wstringstream wss(wround);
	wss << totalRound;
	wss >> wround;
	textTotalRound->setString(WStrToUTF8(wstr + wround));
}


void CSceneBattle::UpdateRoundCountdown(float t)
{
	if (abs(roundRestTime - timeCount) > 1)
		timeCount -= abs(roundRestTime - timeCount);
	roundRestTime -= t;
	--timeCount;	
	if (timeCount < 0)
	{
		///EndTurn
		getScheduler()->unschedule(schedule_selector(CSceneBattle::UpdateRoundCountdown), this);
		return;
	}

	std::wstring wstr = L"回合倒计时：",wtime;
	std::wstringstream wss(wtime);
	wss << timeCount;
	wss >> wtime;
	textRoundCountdown->setString(WStrToUTF8(wstr + wtime));
	log("update %f", t);
}

void CSceneBattle::OnButtonGiveUp()
{
	///
	log("Press GiveUp");
}
void CSceneBattle::OnButtonEndTurn()
{
	///
	log("Press EndTurn");
}
