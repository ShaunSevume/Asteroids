#include "GameUtil.h"
#include "GameWorld.h"
#include "EnemyBullet.h"
#include "Enemy.h"
#include "BoundingSphere.h"

using namespace std;

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/**  Default constructor. */
Enemy::Enemy()
	: GameObject("Enemy"), mThrust(0)
{
	mHp = 5;
	Teleport();
}

/** Construct an enemy with given position, velocity, acceleration, angle, and rotation. */
Enemy::Enemy(GLVector3f p, GLVector3f v, GLVector3f a, GLfloat h, GLfloat r)
	: GameObject("Enemy", p, v, a, h, r), mThrust(0)
{
	mHp = 5;
	Teleport();
}

/** Copy constructor. */
Enemy::Enemy(const Enemy& s)
	: GameObject(s), mThrust(0)
{
	mHp = 5;
	Teleport();
}

/** Destructor. */
Enemy::~Enemy(void)
{
}

// PUBLIC INSTANCE METHODS //

/** Update this enemy. */
void Enemy::Update(int t)
{
	// Call parent update function
	GameObject::Update(t);
	dist = (mTarget->GetPosition() - mPosition);
	if (mAggro) {
		TrackPlayer(); //Hunt the player down!
	}
}

void Enemy::Teleport() {
	mPosition.x = rand() + 60;
	mPosition.y = rand() + 60;
	mPosition.z = 0.0;
}

/** Render this enemy. */
void Enemy::Render(void)
{
	if (mSpaceshipShape.get() != NULL) mSpaceshipShape->Render();

	// If ship is thrusting
	if ((mThrust > 0) && (mThrusterShape.get() != NULL)) {
		mThrusterShape->Render();
	}

	GameObject::Render();
}

/** Track the players. */
void Enemy::TrackPlayer()
{
	float beta = atan2(mTarget->GetPosition().y - mPosition.y, mTarget->GetPosition().x - mPosition.x); //Heading angle in radians
	beta = beta * (180 / M_PI); //Converting to degrees

	mAngle = beta; //Making the enemy face the player.

	int r = rand() % 100; //Generate random number between 0 and 99.
	
	if (dist.length() > 40) { //If the enemy is further than 40...
		// move.
		mVelocity.x = 20 * cos(DEG2RAD * mAngle);
		mVelocity.y = 20 * sin(DEG2RAD * mAngle);
	}else {
		mVelocity.x = 0;
		mVelocity.y = 0;	
		if (r > 95 && mAggro) { //If the random number generated > 95 and the enemy is aggro'd onto the playyer
			Shoot(); //Open fire!!
		}
	}
}

/** Shoot a bullet. */
void Enemy::Shoot(void)
{
	mAngle += rand()%15 - 7.5;
	// Check the world exists
	if (!mWorld) return;
	// Construct a unit length vector in the direction the spaceship is headed
	GLVector3f spaceship_heading(cos(DEG2RAD * mAngle), sin(DEG2RAD * mAngle), 0);
	spaceship_heading.normalize();
	// Calculate the point at the node of the spaceship from position and heading
	GLVector3f bullet_position = mPosition + (spaceship_heading * 4);
	// Calculate how fast the bullet should travel
	float bullet_speed = 30;
	// Construct a vector for the bullet's velocity
	GLVector3f bullet_velocity = mVelocity + spaceship_heading * bullet_speed;
	// Construct a new bullet
	shared_ptr<GameObject> enemy_bullet
	(new EnemyBullet(bullet_position, bullet_velocity, mAcceleration, mAngle, 0, 1750));
	enemy_bullet->SetBoundingShape(make_shared<BoundingSphere>(enemy_bullet->GetThisPtr(), 2.0f));
	enemy_bullet->SetShape(mBulletShape);
	// Add the new bullet to the game world
	mWorld->AddObject(enemy_bullet);
}


bool Enemy::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() != GameObjectType("Bullet")) return false; 
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}


void Enemy::OnCollision(const GameObjectList& objects)
{
	//If this method is triggered (can only be triggered by a bullet), decrement hp and check to see if its 0. If it is, this enemy is dead,
	--mHp;
	Teleport();

	if (mHp == 0) {
		mWorld->FlagForRemoval(GetThisPtr());
	}
}