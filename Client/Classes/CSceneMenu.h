#pragma once

#include "cocos2d.h"
#include "cocos-ext.h"
#include "ui\CocosGUI.h"
#include "cocostudio\CocoStudio.h"

USING_NS_CC_EXT;

class CSceneMenu : public cocos2d::Scene
{
public:
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// implement the "static create()" method manually
	CREATE_FUNC(CSceneMenu);

	Layer *layerMainMenu;
	Layer *layerConfig;
	Layer *layerGameHall;
	Layer *layerCardsGallery;
	Layer *layerTutorial;
	Layer *layerStaff;

	ui::ScrollView *scrollViewTutorial;
	
	ui::ListView *listViewRoomList;

	enum {
		sceneMainMenu,
		sceneGameHall,
		sceneGallery,
		sceneConfig,
		sceneTutorial,
		sceneStaff,
	}currentScene;

	//���˵�
	void OnButtonStart();
	void OnButtonGallery();
	void OnButtonConfig();
	void OnButtonExit();
	//��Ϸ����
	ui::Layout *selectedRoom;
	void OnButtonGameHallJoinGame();
	void OnButtonGameHallReturn();
	//void CSceneMenu::ButtonCreateGame();
	void SelectRoom(ui::Layout *room);
	//����
	void OnButtonConfigReturn();
	void OnButtonConfigTutorial();
	void OnButtonConfigStaff();
	//�������
	void OnButtonGalleryReturn();
	//�̳�
	void OnButtonTutorialReturn();
	//ְԱ��
	void OnButtonStaffReturn();
private:
	void AddRoomList(std::string ID,std::string roomName,std::string playerName);
};