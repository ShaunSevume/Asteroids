#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "GameUtil.h"
#include "GameObject.h"
#include "Shape.h"

class Enemy : public GameObject
{
public:
	Enemy();
	Enemy(GLVector3f p, GLVector3f v, GLVector3f a, GLfloat h, GLfloat r);
	Enemy(const Enemy& s);
	virtual ~Enemy(void);

	virtual void Update(int t);
	virtual void Render(void);

	virtual void TrackPlayer();
	virtual void Shoot(void);
	
	void Teleport();
	void SetSpaceshipShape(shared_ptr<Shape> spaceship_shape) { mSpaceshipShape = spaceship_shape; }
	void SetThrusterShape(shared_ptr<Shape> thruster_shape) { mThrusterShape = thruster_shape; }
	void SetBulletShape(shared_ptr<Shape> bullet_shape) { mBulletShape = bullet_shape; }
	void SetTarget(shared_ptr<GameObject> target) { mTarget = target; }

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);

	void SetAggro(bool cd) { mAggro = cd; }
	void SetHp(int hp) { mHp = hp; }

private:
	float mThrust;
	bool mAggro;
	int mHp;
	GLVector3f dist;
	shared_ptr<GameObject> mTarget;
	shared_ptr<Shape> mSpaceshipShape;
	shared_ptr<Shape> mThrusterShape;
	shared_ptr<Shape> mBulletShape;
	
	
};

#endif