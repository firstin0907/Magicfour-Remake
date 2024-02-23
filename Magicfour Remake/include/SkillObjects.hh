#pragma once

#include "SkillObjectClass.hh"

class SkillObjectSpear : public SkillObjectClass
{
public:
	SkillObjectSpear(int pos_x, int pos_y, int vx, int vy, int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);
	
	static void initialize(class ModelClass* model);

	virtual class ModelClass* GetModel();

private:
	static unique_ptr<class ModelClass> model_;

	float angle_;
};

class SkillObjectBead : public SkillObjectClass
{
public:
	SkillObjectBead(int pos_x, int pos_y, int vx, int vy, int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(class ModelClass* model);

	virtual class ModelClass* GetModel();
private:

	static unique_ptr<class ModelClass> model_;
};

class SkillObjectLeg : public SkillObjectClass
{
public:
	SkillObjectLeg(int pos_x, int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(class ModelClass* model);

	virtual class ModelClass* GetModel();

private:
	static unique_ptr<class ModelClass> model_;
};

class SkillObjectBasic : public SkillObjectClass
{
public:
	SkillObjectBasic(int pos_x, int pos_y, int vx, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(class ModelClass* model);

	virtual class ModelClass* GetModel();

private:
	static unique_ptr<class ModelClass> model_;
};

class SkillObjectShield : public SkillObjectClass
{
public:
	SkillObjectShield(int pos_x, int pos_y,
		int vx, int vy, int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground);

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(class ModelClass* model);

	virtual class ModelClass* GetModel();

private:
	static unique_ptr<class ModelClass> model_;

};