#pragma once

#include "SkillObjectClass.hh"

class SkillObjectSpear : public SkillObjectClass
{
public:
	SkillObjectSpear(int pos_x, int pos_y, int vx, int vy, int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground) override final;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) override final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

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
		const vector<class GroundClass>& ground) override final;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) override final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(class ModelClass* model, class ModelClass* effect_model);

	virtual class ModelClass* GetModel();
	virtual class ModelClass* GetEffectModel();
private:

	static unique_ptr<class ModelClass> model_;
	static unique_ptr<class ModelClass> effect_model_;
};

class SkillObjectLeg : public SkillObjectClass
{
public:
	SkillObjectLeg(int pos_x, int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground) override final;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) override final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(class ModelClass* model);

	virtual class ModelClass* GetModel();

private:
	static unique_ptr<class ModelClass> model_;
};

class SkillObjectBasic : public SkillObjectClass
{
public:
	SkillObjectBasic(int pos_x, int pos_y, int vx,
		int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground) override final;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) override final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

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
		const vector<class GroundClass>& ground) override final;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) override final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(class ModelClass* model);

	virtual class ModelClass* GetModel();

private:
	static unique_ptr<class ModelClass> model_;

};

class SkillObjectGuardian : public SkillObjectClass
{
public:
	SkillObjectGuardian();

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground) override final {};

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) override final { return true;  };

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(class ModelClass* model);

	virtual class ModelClass* GetModel();

	inline void SetPosition(int x, int y)
	{
		position_ = Vector2d(x, y);
	}

private:
	static unique_ptr<class ModelClass> model_;

};