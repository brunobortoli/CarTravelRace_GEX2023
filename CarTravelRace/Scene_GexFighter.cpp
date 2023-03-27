#include "Scene_GexFighter.h"
#include "Entity.h"

#include <fstream>
#include <iostream>
#include <random>
#include "MusicPlayer.h"
#include "SoundPlayer.h"

namespace {
	std::random_device rd;
	std::mt19937 rng(rd());
}


Scene_GexFighter::Scene_GexFighter(GameEngine* gameEngine, const std::string& configPath)
	: Scene(gameEngine), m_worldView(gameEngine->window().getDefaultView()), m_worldBounds({ 0, 0 }, { 0, 0 }) {

	loadFromFile(configPath);

	m_spawnPosition = sf::Vector2f(m_worldView.getSize().x / 2.f,
		m_worldBounds.height - m_worldView.getSize().y / 2.f);

	m_worldView.setCenter(m_spawnPosition);

	registerActions();
	spawnPlayer();
	//spawnEnemy("car01", m_spawnPosition + sf::Vector2f(50.f, 100.f));
	//spawnEnemy("car02", m_spawnPosition + sf::Vector2f(50.f, 100.f));
	//spawnEnemy("car01", m_spawnPosition + sf::Vector2f(0.f, -500.f));
	// spawnEnemy("Avenger", m_spawnPosition + sf::Vector2f(0.f, -500.f));
	// spawnEnemy("Avenger", m_spawnPosition + sf::Vector2f(300.f, -500.f));
	// spawnEnemy("Avenger", m_spawnPosition + sf::Vector2f(-300.f, -500.f));

}


void Scene_GexFighter::loadFromFile(const std::string& configPath) {
	std::ifstream config(configPath);
	if (config.fail()) {
		std::cerr << "Open file " << configPath << " failed\n";
		config.close();
		exit(1);
	}


	std::string token{ "" };
	config >> token;
	while (config) {
		if (token == "#") {
			std::string tmp;
			std::getline(config, tmp);
			std::cout << tmp << "\n";
		}
		else if (token == "World") {
			config >> m_worldBounds.width >> m_worldBounds.height;
		}
		else if (token == "ScrollSpeed") {
			//config >> m_scrollSpeed;
			m_scrollSpeed = 0;
		}
		else if (token == "PlayerSpeed") {
			config >> m_playerSpeed;
		}
		else if (token == "EnemySpeed") {
			config >> m_enemySpeed;
		}
		else if (token == "BulletSpeed") {
			config >> m_bulletSpeed;
		}
		else if (token == "MissileSpeed") {
			config >> m_missileSpeed;
		}
		else if (token == "FireInterval") {
			float interval;
			config >> interval;
			m_fireInterval = sf::seconds(interval);
		}
		else if (token == "Bkg") {
			std::string name;
			sf::Vector2f pos;
			config >> name >> pos.x >> pos.y;

			auto e = m_entityManager.addEntity("bkg");

			auto& sprite
				= e->addComponent<CSprite>(m_game->assets().getTexture(name)).sprite;

			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(pos);
			if (name == "Finish") {
				m_finishPoint = pos.y;
				auto width = sprite.getLocalBounds().width;
				sprite.setOrigin(width, 0.f);
				sprite.setRotation(180);
			}

		}
		else if (token == "Entities") {
			sf::IntRect tr;
			std::string name;
			config >> name >> tr.left >> tr.top >> tr.width >> tr.height;
			m_textRects[name] = tr;
		}
		else if (token == "Directions") {
			std::string name;
			config >> name;
			float time;

			// auto pilots have 5 legs
			for (int i{ 0 }; i < 5; ++i) {
				config >> m_enemyConfig[name].dirs[i];
				config >> time;
				m_enemyConfig[name].times[i] = sf::seconds(time);
			}
		}
		config >> token;
	}
	config.close();
}


void Scene_GexFighter::init(const std::string& configPath) {

}


void Scene_GexFighter::keepEntitiesInBounds() {

}


void Scene_GexFighter::onEnd() {

}


void Scene_GexFighter::sMovement(sf::Time dt) {
	playerMovement();

	// move all objects
	for (auto e : m_entityManager.getEntities()) {
		if (e->hasComponent<CTransform>()) {
			auto& tfm = e->getComponent<CTransform>();

			tfm.pos += tfm.vel * dt.asSeconds();
			tfm.rot += tfm.rotVel * dt.asSeconds();
		}
	}
}


void Scene_GexFighter::playerMovement() {

	// no movement if player is dead
	if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
		return;

	// no movement if player won
	if (m_player->hasComponent<CTransform>()) {
		auto playerY = m_player->getComponent<CTransform>().pos.y;
		if (playerY <= m_finishPoint)
			m_playerWon = true;
	}

	// player movement
	sf::Vector2f pv;
	auto& pInput = m_player->getComponent<CInput>();
	if (pInput.left) pv.x -= 1;
	if (pInput.right) pv.x += 1;
	if (pInput.up) pv.y -= 1;
	if (pInput.down) pv.y += 1;
	pv = normalize(pv);
	m_player->getComponent<CTransform>().vel = m_playerSpeed * pv;
}


void Scene_GexFighter::sCollisions() {
	checkMissileCollision();
	checkBulletCollision();
	checkPlaneCollision();
	checkPickupCollision();
}


void Scene_GexFighter::checkIfDead(NttPtr e) {

	// check for planes that need to be destroyed
	if (e->hasComponent<CHealth>()) {
		if (e->getComponent<CHealth>().hp <= 0) {
			e->addComponent<CAnimation>(m_game->assets().getAnimation("explosion"));
			e->getComponent<CTransform>().vel = sf::Vector2f(0.f, 0.f);
			e->addComponent<CState>().state = "dead";
			e->removeComponent<CCollision>();

			if (e->getTag() == "player")
				m_playerLost = true;
			else
				spawnPickup(e);

			SoundPlayer::getInstance().play("Explosion1");
		}
	}
}

void Scene_GexFighter::spawnPickup(NttPtr e)
{
	if (e->hasComponent<CTransform>()) {

		auto spawnPos = e->getComponent<CTransform>().pos;
		std::uniform_int_distribution type(1, 4);
		auto chosenPickup = type(rng);
		std::string texture;
		std::string group;

		switch (chosenPickup) {
		case 1:
			texture = "HealthRefill";
			group = "healthPickup";
			break;
		case 2:
			texture = "MissileRefill";
			group = "missilePickup";
			break;
		case 3:
			texture = "FireRate";
			group = "fireRatePickup";
			break;
		case 4:
			texture = "FireSpread";
			group = "fireSpreadPickup";
			break;
		}

		auto pickup = m_entityManager.addEntity(group);
		pickup->addComponent<CCollision>(20.f);
		pickup->addComponent<CTransform>(spawnPos, sf::Vector2f{ 0.f, 0.f });

		auto textureFile = m_game->assets().getAnimation(texture);
		pickup->addComponent<CAnimation>(textureFile);
	}
}

int Scene_GexFighter::countEnemiesOnView()
{
	auto viewBounds = getViewBounds();
	int count = 0;
	for (auto enemyCar : m_entityManager.getEntities("enemy")) {
		auto enemyPos = enemyCar->getComponent<CTransform>().pos;

		if (enemyPos.x > viewBounds.left &&
			enemyPos.x < viewBounds.left + viewBounds.width &&
			enemyPos.y > viewBounds.top &&
			enemyPos.y < viewBounds.top + viewBounds.height
			)
			count++;
	}

	return count;
}


void Scene_GexFighter::checkPlaneCollision() {// check for plane collision

	if (m_player->hasComponent<CCollision>()) {
		auto pPos = m_player->getComponent<CTransform>().pos;
		auto pCr = m_player->getComponent<CCollision>().radius;

		for (auto e : m_entityManager.getEntities("enemy")) {
			if (e->hasComponent<CTransform>() && e->hasComponent<CCollision>()) {
				auto ePos = e->getComponent<CTransform>().pos;
				auto eCr = e->getComponent<CCollision>().radius;

				// planes have collided
				if (dist(ePos, pPos) < (eCr + pCr)) {
					auto& pHP = m_player->getComponent<CHealth>().hp;
					auto& eHP = e->getComponent<CHealth>().hp;

					// however many HP the plane has left,
					// that's how much damage it inflicts on other plane
					int tmpHP = pHP;
					pHP -= eHP;
					eHP -= tmpHP;

					checkIfDead(e);
					checkIfDead(m_player);
				}

			}
		}
	}
}

void Scene_GexFighter::checkPickupCollision()
{
	auto playerPos = m_player->getComponent<CTransform>().pos;
	auto playerCR = m_player->getComponent<CCollision>().radius;
	for (auto healthPickup : m_entityManager.getEntities("healthPickup")) {
		auto pickupPos = healthPickup->getComponent<CTransform>().pos;
		auto pickupCR = healthPickup->getComponent<CCollision>().radius;

		if (dist(playerPos, pickupPos) < (playerCR + pickupCR)) {
			auto& playerHealth = m_player->getComponent<CHealth>().hp;
			playerHealth += 30;

			healthPickup->destroy();
			SoundPlayer::getInstance().play("CollectPickup");
		}
	}

	for (auto missilePickup : m_entityManager.getEntities("missilePickup")) {
		auto pickupPos = missilePickup->getComponent<CTransform>().pos;
		auto pickupCR = missilePickup->getComponent<CCollision>().radius;

		if (dist(playerPos, pickupPos) < (playerCR + pickupCR)) {
			auto& missleCount = m_player->getComponent<CMissiles>().missileCount;
			missleCount += 2;

			missilePickup->destroy();
			SoundPlayer::getInstance().play("CollectPickup");
		}
	}

	for (auto fireRatePickup : m_entityManager.getEntities("fireRatePickup")) {
		auto pickupPos = fireRatePickup->getComponent<CTransform>().pos;
		auto pickupCR = fireRatePickup->getComponent<CCollision>().radius;

		if (dist(playerPos, pickupPos) < (playerCR + pickupCR)) {
			auto& fireRate = m_player->getComponent<CGun>().fireRate;
			fireRate += 5;

			fireRatePickup->destroy();
			SoundPlayer::getInstance().play("CollectPickup");
		}
	}

	for (auto fireSpreadPickup : m_entityManager.getEntities("fireSpreadPickup")) {
		auto pickupPos = fireSpreadPickup->getComponent<CTransform>().pos;
		auto pickupCR = fireSpreadPickup->getComponent<CCollision>().radius;

		if (dist(playerPos, pickupPos) < (playerCR + pickupCR)) {
			auto& spreadLevel = m_player->getComponent<CGun>().spreadLevel;
			if (spreadLevel < 3)
				spreadLevel++;

			fireSpreadPickup->destroy();
			SoundPlayer::getInstance().play("CollectPickup");
		}
	}
}


void Scene_GexFighter::checkBulletCollision() {

	// Player Bullets
	for (auto bullet : m_entityManager.getEntities("playerBullet")) {
		if (bullet->hasComponent<CTransform>() && bullet->hasComponent<CCollision>()) {
			auto bPos = bullet->getComponent<CTransform>().pos;
			auto bCr = bullet->getComponent<CCollision>().radius;

			for (auto e : m_entityManager.getEntities("enemy")) {
				if (e->hasComponent<CTransform>() && e->hasComponent<CCollision>()) {
					auto ePos = e->getComponent<CTransform>().pos;
					auto eCr = e->getComponent<CCollision>().radius;

					if (dist(ePos, bPos) < (eCr + bCr)) {
						e->getComponent<CHealth>().hp -= 10;
						bullet->destroy();
						checkIfDead(e);
					}
				}
			}
		}
	}


	// Enemy Bullets
	if (m_player->hasComponent<CCollision>()) {
		auto pPos = m_player->getComponent<CTransform>().pos;
		auto pCr = m_player->getComponent<CCollision>().radius;

		for (auto bullet : m_entityManager.getEntities("enemyBullet")) {
			if (bullet->hasComponent<CTransform>() && bullet->hasComponent<CCollision>()) {
				auto bPos = bullet->getComponent<CTransform>().pos;
				auto bCr = bullet->getComponent<CCollision>().radius;

				if (dist(pPos, bPos) < (pCr + bCr)) {
					m_player->getComponent<CHealth>().hp -= 10;
					bullet->destroy();
					checkIfDead(m_player);
				}
			}
		}
	}
}


void Scene_GexFighter::checkMissileCollision() {// missiles
	for (auto m : m_entityManager.getEntities("missile")) {
		if (m->hasComponent<CTransform>() && m->hasComponent<CCollision>()) {
			auto mPos = m->getComponent<CTransform>().pos;
			auto mCr = m->getComponent<CCollision>().radius;

			for (auto e : m_entityManager.getEntities("enemy")) {
				if (e->hasComponent<CTransform>() && e->hasComponent<CCollision>()) {
					auto ePos = e->getComponent<CTransform>().pos;
					auto eCr = e->getComponent<CCollision>().radius;

					if (dist(ePos, mPos) < (eCr + mCr)) {
						e->getComponent<CHealth>().hp = -1;
						m->destroy();
						checkIfDead(e);
					}
				}
			}
		}
	}
}


void Scene_GexFighter::sUpdate(sf::Time dt) {

}


void Scene_GexFighter::registerActions() {

	//TODO register action FIRE to fire the gun

	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "BACK");
	registerAction(sf::Keyboard::Q, "QUIT");

	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");

	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::W, "ACCELERATE");
	registerAction(sf::Keyboard::Up, "ACCELERATE");
	registerAction(sf::Keyboard::S, "BREAK");
	registerAction(sf::Keyboard::Down, "BREAK");

	registerAction(sf::Keyboard::Space, "FIRE");
	registerAction(sf::Keyboard::M, "LAUNCH");

}


void Scene_GexFighter::spawnPlayer() {
	m_player = m_entityManager.addEntity("player");
	//m_player->addComponent<CTransform>(
	//	m_spawnPosition,
	//	sf::Vector2f(0.f, 0.f),
	//	0, 0);

	auto vb = getViewBounds();
	m_player->addComponent<CTransform>(
		sf::Vector2f{ vb.width / 2.f, vb.top + vb.height - 46.f },
		sf::Vector2f(0.f, 0.f),
		0, 0);

	//m_player->addComponent<CAnimation>(m_game->assets().getAnimation("EagleStr"));
	m_player->addComponent<CSprite>(m_game->assets().getTexture("car"));
	//m_player->addComponent<CState>("straight");
	m_player->addComponent<CCollision>(20);
	m_player->addComponent<CInput>();
	//m_player->addComponent<CMissiles>();
	m_player->addComponent<CHealth>().hp = 100;
	//auto& gun = m_player->addComponent<CGun>();

}


void Scene_GexFighter::drawAABB() {

	for (auto e : m_entityManager.getEntities()) {

		// draw the world
		if (e->getComponent<CCollision>().has) {

			auto& tfm = e->getComponent<CTransform>();
			auto cr = e->getComponent<CCollision>().radius;

			sf::CircleShape cir(cr);
			cir.setOutlineThickness(2);
			cir.setOutlineColor(sf::Color::Red);
			cir.setFillColor(sf::Color::Transparent);

			cir.setPosition(tfm.pos);
			centerOrigin(cir);

			m_game->window().draw(cir);
		}
	}
}


void Scene_GexFighter::adjustPlayer() {
	auto vb = getViewBounds();

	auto& pos = m_player->getComponent<CTransform>().pos;
	auto cr = m_player->getComponent<CCollision>().radius;
	//
	//pos.x = std::max(pos.x, vb.left + cr);
	//pos.x = std::min(pos.x, vb.left + vb.width - cr);
	pos.y = std::max(pos.y, vb.top + cr);
	pos.y = std::min(pos.y, vb.top + vb.height - 46.f);

	pos.x = std::max(pos.x, ROAD_LEFT_POS + cr);
	pos.x = std::min(pos.x, ROAD_RIGHT_POS - cr);
}


void Scene_GexFighter::checkPlayerState() {// set the player state
	if (m_player->hasComponent<CState>()) {

		auto xVel = m_player->getComponent<CTransform>().vel.x;
		std::string newState = "straight";
		if (xVel < -0.2f) newState = "left";
		if (xVel > 0.2f) newState = "right";

		auto& state = m_player->getComponent<CState>().state;
		if (state != "dead") {
			auto& state = m_player->getComponent<CState>().state;
			if (newState != state) { // only if the state has changed, change the animation
				state = newState;
				if (state == "straight")
					m_player->addComponent<CAnimation>(m_game->assets().getAnimation("EagleStr"));
				if (state == "left")
					m_player->addComponent<CAnimation>(m_game->assets().getAnimation("EagleLft"));
				if (state == "right")
					m_player->addComponent<CAnimation>(m_game->assets().getAnimation("EagleRgt"));
			}
		}
	}
}


sf::FloatRect Scene_GexFighter::getViewBounds() {
	sf::FloatRect bounds;
	bounds.left = m_worldView.getCenter().x - m_worldView.getSize().x / 2.f;
	bounds.top = m_worldView.getCenter().y - m_worldView.getSize().y / 2.f;
	bounds.width = m_worldView.getSize().x;
	bounds.height = m_worldView.getSize().y;
	return bounds;
}


void Scene_GexFighter::update(sf::Time dt) {

	if (m_playerWon || m_playerLost)
		return;

	if (m_isPaused)
		return;

	m_entityManager.update();
	m_worldView.move(0.f, m_scrollSpeed * dt.asSeconds() * -1);
	//m_worldView.move(0.f, 0 * dt.asSeconds() * -1);

	adjustPlayer();
	//checkPlayerState();
	sMovement(dt);
	//sCollisions();
	sGunUpdate(dt);
	sAnimation(dt);
	sGuideMissiles(dt);
	sAutoPilot(dt);
	spawnEnemies();
	SoundPlayer::getInstance().removeStoppedSounds();
}


void Scene_GexFighter::sDoAction(const Action& action) {

	// On Key Press
	if (action.type() == "START") {

		if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
		else if (action.name() == "QUIT") { m_game->quitLevel(); }
		else if (action.name() == "BACK") { m_game->backLevel(); }

		else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

		// Player control
		else if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = true; }
		else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = true; }
		//else if (action.name() == "UP") { m_player->getComponent<CInput>().up = true; }
		//else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = true; }

		// firing weapons
		else if (action.name() == "FIRE") { fireBullet(); }
		else if (action.name() == "LAUNCH") { fireMissile(); }
		else if (action.name() == "ACCELERATE") { m_scrollSpeed += 10; }
		else if (action.name() == "BREAK") {
			m_scrollSpeed -= 10;
			if (m_scrollSpeed <= 0)
				m_scrollSpeed = 0;
		}
	}

	// on Key Release
	else if (action.type() == "END") {
		if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = false; }
		else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = false; }
		else if (action.name() == "UP") { m_player->getComponent<CInput>().up = false; }
		else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = false; }
	}

}


void Scene_GexFighter::sRender() {

	m_game->window().setView(m_worldView);

	// draw world
	auto bgColor = sf::Color(100, 100, 255);
	if (m_isPaused)
		bgColor = sf::Color(150, 50, 255);

	m_game->window().clear(bgColor);

	// draw bkg first
	for (auto e : m_entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			auto& sprite = e->getComponent<CSprite>().sprite;

			m_game->window().draw(sprite);
		}
	}

	for (auto e : m_entityManager.getEntities()) {

		// draw all entities with animation component
		if (e->getComponent<CAnimation>().has) {
			auto& tfm = e->getComponent<CTransform>();
			auto& anim = e->getComponent<CAnimation>().animation;
			anim.getSprite().setPosition(tfm.pos);
			anim.getSprite().setRotation(tfm.rot);
			m_game->window().draw(anim.getSprite());

			// draw HP
			static sf::Text text("HP: ", m_game->assets().getFont("Arial"), 15);
			if (e->hasComponent<CHealth>()) {
				int hp = e->getComponent<CHealth>().hp;
				std::string str = "HP: " + std::to_string(hp);
				text.setString(str);
				centerOrigin(text);

				sf::Vector2f offset(0.f, 40.f);
				if (e->getTag() == "enemy")
					offset *= -1.f;
				text.setPosition(tfm.pos + offset);
				m_game->window().draw(text);
			}

			// draw ammo count if missiles
			if (e->hasComponent<CMissiles>()) {
				int count = e->getComponent<CMissiles>().missileCount;
				std::string str = "M: " + std::to_string(count);
				text.setString(str);
				centerOrigin(text);

				sf::Vector2f offset(0.f, 55.f);
				if (e->getTag() == "enemy")
					offset *= -1.f;
				text.setPosition(tfm.pos + offset);
				m_game->window().draw(text);
			}
		}

		// draw entities with CSprite component
		if (e->getComponent<CSprite>().has && e->getTag() != "bkg") {
			auto& tfm = e->getComponent<CTransform>();
			auto& spr = e->getComponent<CSprite>().sprite;
			spr.setPosition(tfm.pos);
			spr.setRotation(tfm.rot);
			m_game->window().draw(spr);
		}
	}

	if (m_isPaused) {
		sf::Text paused("PAUSED", m_game->assets().getFont("Star"), 128);
		centerOrigin(paused);
		auto bounds = getViewBounds();
		paused.setPosition(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
		m_game->window().draw(paused);
	}

	if (m_playerWon) {
		sf::Text msg("You Win", m_game->assets().getFont("Star"), 128);
		centerOrigin(msg);
		auto bounds = getViewBounds();
		msg.setPosition(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
		m_game->window().draw(msg);
	}

	if (m_playerLost) {
		sf::Text msg("You Lose", m_game->assets().getFont("Star"), 128);
		centerOrigin(msg);
		auto bounds = getViewBounds();
		msg.setPosition(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
		m_game->window().draw(msg);
	}

	// draw bounding boxes
	if (m_drawAABB) {
		drawAABB();
	}


}


void Scene_GexFighter::sAnimation(sf::Time dt) {

	for (auto e : m_entityManager.getEntities()) {

		// draw the world
		if (e->getComponent<CAnimation>().has) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);
			if (anim.animation.hasEnded())
				e->destroy();
		}
	}
}


void Scene_GexFighter::fireBullet() {
	m_player->getComponent<CGun>().isFiring = true;
}


void Scene_GexFighter::createBullet(sf::Vector2f pos, bool isEnemy) {
	float speed = (isEnemy) ? m_bulletSpeed : -m_bulletSpeed;

	auto bullet = m_entityManager.addEntity(isEnemy ? "enemyBullet" : "playerBullet");
	bullet->addComponent<CTransform>(pos, sf::Vector2f(0.f, speed));
	bullet->addComponent<CAnimation>(m_game->assets().getAnimation("Bullet"));
	bullet->addComponent<CCollision>(3);

	if (isEnemy)
		SoundPlayer::getInstance().play("EnemyGunfire");
	else
		SoundPlayer::getInstance().play("AlliedGunfire");
}


void Scene_GexFighter::sGunUpdate(sf::Time dt) {
	for (auto e : m_entityManager.getEntities()) {
		if (e->hasComponent<CGun>()) {

			//
			// every time
			//
			auto& gun = e->getComponent<CGun>();
			bool isEnemy = (e->getTag() == "enemy");
			if (isEnemy)
				gun.isFiring = true;

			gun.countdown -= dt;

			//
			// when firing
			//
			if (gun.isFiring && gun.countdown < sf::Time::Zero) {
				gun.isFiring = false;
				gun.countdown = m_fireInterval / (1.f + gun.fireRate);

				auto pos = e->getComponent<CTransform>().pos;
				switch (gun.spreadLevel) {
				case 1:
					createBullet(pos + sf::Vector2f(0.f, isEnemy ? 35.f : -35.f), isEnemy);
					break;

				case 2:
					createBullet(pos + sf::Vector2f(-20.f, 0.f), isEnemy);
					createBullet(pos + sf::Vector2f(20.f, 0.f), isEnemy);
					break;

				case 3:
					createBullet(pos + sf::Vector2f(0.f, -35.f), isEnemy);
					createBullet(pos + sf::Vector2f(-20.f, 0.f), isEnemy);
					createBullet(pos + sf::Vector2f(20.f, 0.f), isEnemy);
					break;

				default:
					std::cerr << "Bad spread level firing gun\n";
					break;
				}

			}
		}
	}
}


void Scene_GexFighter::fireMissile() {

	if (m_player->hasComponent<CMissiles>()) {
		size_t& ammo = m_player->getComponent<CMissiles>().missileCount;
		if (ammo > 0) {
			ammo -= 1;
			auto pos = m_player->getComponent<CTransform>().pos;

			auto missile = m_entityManager.addEntity("missile");
			missile->addComponent<CTransform>(
				pos + sf::Vector2f(0.f, -60.f),
				sf::Vector2f(0.f, -m_missileSpeed));
			missile->addComponent<CAnimation>(m_game->assets().getAnimation("Missile"));
			missile->addComponent<CCollision>(14);

			SoundPlayer::getInstance().play("LaunchMissile");

			//std::cout << "origin: " << missile->getComponent<CAnimation>().animation.getSprite().getOrigin() << "\n";
		}
	}
}


sf::Vector2f Scene_GexFighter::findClosestEnemy(sf::Vector2f mPos) {
	float closest = std::numeric_limits<float>::max();
	sf::Vector2f posClosest{ 0.f, 0.f };
	for (auto e : m_entityManager.getEntities("enemy")) {
		if (e->getComponent<CTransform>().has) {
			auto ePos = e->getComponent<CTransform>().pos;
			float distToEnemy = dist(mPos, ePos);
			if (distToEnemy < closest) {
				closest = distToEnemy;
				posClosest = ePos;
			}
		}
	}
	return posClosest;
}


void Scene_GexFighter::sGuideMissiles(sf::Time dt) {

	const float approachRate = 800.f;
	for (auto e : m_entityManager.getEntities("missile")) {

		if (e->getComponent<CTransform>().has) {
			auto& tfm = e->getComponent<CTransform>();
			auto ePos = findClosestEnemy(tfm.pos);

			auto targetDir = normalize(ePos - tfm.pos);
			tfm.vel = m_missileSpeed * normalize(approachRate * dt.asSeconds() * targetDir + tfm.vel);
			tfm.rot = bearing(tfm.vel) + 90;
		}
	}
}


void Scene_GexFighter::spawnEnemy(std::string type, sf::Vector2f pos) {

	auto vel = sf::Vector2f(0.f, m_enemySpeed);
	//float rotation = 180.f;
	float rotation = 0.f;

	auto opponentCar = m_entityManager.addEntity("enemy");
	opponentCar->addComponent<CTransform>(pos, vel, rotation);

	opponentCar->addComponent<CSprite>(m_game->assets().getTexture(type));

	opponentCar->addComponent<CCollision>(20);
	opponentCar->addComponent<CHealth>(100);

	auto& ap = opponentCar->addComponent<CAutoPilot>();
	ap.bearings = m_enemyConfig[type].dirs;
	ap.lengths = m_enemyConfig[type].times;

	//if (type == "Avenger")
	//	enemyPlane->addComponent<CGun>();
}


//void Scene_GexFighter::spawnEnemies() {
//	std::uniform_int_distribution<int> numberOfPlanes(1, 1);
//	std::uniform_int_distribution<int> typeOfPlane(1, 9);
//	std::exponential_distribution<float> nextSpawnPoint(1.f / 250.f);
//
//	auto bounds = getViewBounds();
//	static float next_spawn_point{ bounds.top - 100 };
//
//	if (next_spawn_point > bounds.top - 100) {
//		int num = numberOfPlanes(rng);
//		int carNumber = typeOfPlane(rng);
//		std::string carType = "car0" + std::to_string(carNumber);
//		spawnEnemies(carType, next_spawn_point, num);
//		next_spawn_point = bounds.top - 200.f - nextSpawnPoint(rng);
//	}
//}

void Scene_GexFighter::spawnEnemies() {

	std::uniform_int_distribution<int> typeOfCar(1, 9);
	std::uniform_int_distribution<int> generate(1, 1000);
	int left = (int)ROAD_LEFT_POS + 20;
	int right = (int)ROAD_RIGHT_POS - 20;
	std::uniform_int_distribution<int> xSpawnPoint(left, right);
	auto bounds = getViewBounds();

	auto pos = sf::Vector2f(xSpawnPoint(rng), bounds.top - 100.f);
	auto ready = generate(rng);
	if ((ready % 100 == 0) && (countEnemiesOnView() < 4)) {
		int carNumber = typeOfCar(rng);
		std::string carType = "car0" + std::to_string(carNumber);
		spawnEnemy(carType, pos);

	}

}


void Scene_GexFighter::sAutoPilot(const sf::Time& dt) {// autopilot enemties
	for (auto e : m_entityManager.getEntities("enemy")) {
		if (e->hasComponent<CAutoPilot>()) {
			auto& ai = e->getComponent<CAutoPilot>();
			ai.countdown -= dt;
			if (ai.countdown < sf::Time::Zero)
			{
				ai.countdown = ai.lengths[ai.currentLeg];
				ai.currentLeg = (ai.currentLeg + 1) % ai.legs;

				auto& tfm = e->getComponent<CTransform>();
				tfm.vel = length(tfm.vel) * uVecFromBearing(90 + ai.bearings[ai.currentLeg]);
			}
		}
	}
}

