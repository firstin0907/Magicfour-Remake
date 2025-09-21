#pragma once

#include "SkillObjectClass.hh"

#include <string>

class SkillObjectSpear : public SkillObjectClass
{
public:
	SkillObjectSpear(int pos_x, int pos_y, int vx, int vy, int skill_level, time_t created_time);

	// Move instance as time goes by.
	virtual void FrameMove(time_t curr_time, time_t time_delta,
		const vector<class GroundClass>& ground) override final;

	// Should be called after processing any collision with monsters. 
	virtual bool Frame(time_t curr_time, time_t time_delta) override final;

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);
	
	static void initialize(const std::string& model_name);

private:
	static std::string model_name_;

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

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(const std::string& model_name, const std::string& effect_model_name);

	virtual std::string GetEffectModel();
private:

	static std::string model_name_;
	static std::string effect_model_name_;
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

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(const std::string& model_name);

private:
	static std::string model_name_;
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

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(const std::string& model_name);
	

private:
	static std::string model_name_;
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

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(const std::string& model_name);
	

private:
	static std::string model_name_;

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

	// Render this instance to game scene.
	virtual void Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
		ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const final;

	// Should be called when this instance is collided with any valid(live) monster.
	virtual bool OnCollided(class MonsterClass* monster, time_t collided_time);

	virtual XMMATRIX GetGlobalShapeTransform(time_t curr_time);

	static void initialize(const std::string& model_name);


	inline void SetPosition(int x, int y)
	{
		position_ = Vector2d(x, y);
	}

private:
	static std::string model_name_;

};