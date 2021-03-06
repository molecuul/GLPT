#include "GLPT.h"
#include "Player.h"
#include "Properties.h"

#define max(x, y) ((x > y) ? x : y)
#define min(x, y) ((x < y) ? x : y)

void Player::EventCreate(EntityLoadData* data) {
	SetIdentity("ins_player");
	SetEventDepth(-5);

	if (!data) {
		PostWarning("Player created with no initial data. Destroying.");
		Destroy();
		return;
	}

	PhysBodyDesc player_body_desc;

	player_body_desc.angle=dtof(data,"angle");
	player_body_desc.dynamic=true;
	player_body_desc.height=0.9f;
	player_body_desc.width=0.5f;
	player_body_desc.weight=0.3f;
	player_body_desc.x=dtof(data,"x");
	player_body_desc.y=dtof(data,"y");

	phys_object=GLPT_physics->CreateBody(this,player_body_desc);

	BasicVertex vertices[6];
	BasicVertex::make_rectangle(vertices,1.0f,1.0f);

	draw_object.Load(vertices,6);

	ani=new Animation();
	ani->CreateFromFile("resource_player.ani");
	ani->SetAnimationState("Idle");

	draw_object.SetAnimation(ani);

	initial_player_spawn=true;
	initial_position=phys_object->GetPosition();

	
}

void Player::EventDraw(void) {

	static float x=0.0f,y=0.0f,angle=0.0f;

	x=phys_object->GetPosition().x;
	y=phys_object->GetPosition().y;
	angle=phys_object->GetAngle();

	draw_object.Draw(x,y,angle);

}

void Player::EventStep(void) {
	// Movement control here.

	if (initial_player_spawn) {
		/*Overlay* overlay_entity=(Overlay*) GLPT_iterator->GetByIdentity("global_ent_overlay");
		if (overlay_entity) {
			overlay_entity->SetFadeData(1.0f,1.0f,1.0f,1.0f);
		}*/
		
		initial_player_spawn=false;
	}

	phys_object->SetFixedRotation(true);

	bool player_can_jump=false;
	bool player_can_move=false;

	b2Vec2 player_position=phys_object->GetPosition();
	b2Vec2 player_velocity=phys_object->GetLinearVelocity();
	b2Vec2 player_dimension=b2Vec2(0.5f,0.9f);

	float player_decel_rate=1.06;

	/* static Overlay* overlay_entity=(Overlay*) GLPT_iterator->GetByIdentity("global_ent_overlay");
	if (!overlay_entity) overlay_entity=(Overlay*) GLPT_iterator->GetByIdentity("global_ent_overlay"); */

	static Property* prop_entity=(Property*) GLPT_iterator->GetByIdentity("property");
	if (!prop_entity) prop_entity=(Property*) GLPT_iterator->GetByIdentity("property");

	float death_mark=(prop_entity) ? prop_entity->GetDeathPosition() : -25.0f;

	player_can_jump=Grounded();

	// Done with 'player_can_jump'.
	// Now, check for 'player_can_move'.

	// For now, we can let the player move in midair. Remember to use 'player_can_jump', and just slow down the movement.

	player_can_move=player_can_jump;

	// Now, for literal movement and animation controls.

	// Jumping.
	if (GLPT_input->KD(GLFW_KEY_UP) && player_can_jump) {
		phys_object->SetLinearVelocity(b2Vec2(player_velocity.x,9.5f));
		player_velocity=phys_object->GetLinearVelocity();
		ani->SetAnimationState("BeginJump");

		player_can_jump=false;
	}

	if (!GLPT_input->KD(GLFW_KEY_UP) && player_velocity.y>0) {
		player_velocity.y/=1.05;
		phys_object->SetLinearVelocity(player_velocity);
	}

	// LR Movement.

	if (player_can_move) {
		if (GLPT_input->KD(GLFW_KEY_RIGHT)) {
			if (player_velocity.x < 6) 
				player_velocity.x=min(player_velocity.x+=0.5f,6);
			phys_object->SetLinearVelocity(player_velocity);
			draw_object.Flip(false);
			if (player_can_jump && ani->GetAnimationState()!="Walking") ani->SetAnimationState("Walking");
		}
		if (GLPT_input->KD(GLFW_KEY_LEFT)) {
			player_velocity.x=max(player_velocity.x-0.5f,-6.0f);
			phys_object->SetLinearVelocity(player_velocity);
			draw_object.Flip(true);
			if (player_can_jump && ani->GetAnimationState()!="Walking") ani->SetAnimationState("Walking");
		}
	}

	// Animation control.

	if (!GLPT_input->KD(GLFW_KEY_LEFT) && !GLPT_input->KD(GLFW_KEY_RIGHT) &&
	ani->GetAnimationState()=="Walking" && player_can_jump) {
		ani->SetAnimationState("Idle");
	}

	if (!GLPT_input->KD(GLFW_KEY_LEFT) && !GLPT_input->KD(GLFW_KEY_RIGHT) &&
	ani->GetAnimationState()=="Idle" && player_can_jump) {
		player_velocity.x/=player_decel_rate;
		phys_object->SetLinearVelocity(player_velocity);
	}

	if (ani->GetAnimationState()=="EndJump" && player_can_jump && !player_velocity.y) {
		ani->SetAnimationState("Idle");
	}

	if (player_velocity.y < 0 && !player_can_jump && ani->GetAnimationState()!="EndJump") {
		ani->SetAnimationState("EndJump");
	}

	/*
	if (player_position.y < death_mark) {
		if (overlay_entity) {
			overlay_entity->SetFadeData(1.0f,0.0f,0.0f,(25-abs((death_mark-25)-player_position.y))/25.0f);
		}
	} else {
		float over_r,over_g,over_b,over_a;
		overlay_entity->GetFadeData(&over_r,&over_g,&over_b,&over_a);

		if (over_r==1.0f && over_g==1.0f && over_b==1.0f && over_a) {
			overlay_entity->SetFadeData(1.0f,1.0f,1.0f,over_a/1.02f);
		}
	}
	*/

	if (player_position.y <= death_mark-25.0f) {
		player_position=initial_position;
		phys_object->SetTransform(initial_position,0.0f);
		// if (overlay_entity) overlay_entity->SetFadeData(1.0f,1.0f,1.0f,1.0f);
	}
}

void Player::EventDestroy(void) {

}

void Player::GetPosition(float* x,float* y) {

	b2Vec2 pos=phys_object->GetPosition();

	if (x) {
		*x=pos.x;
	}

	if (y) {
		*y=pos.y;
	}

}

bool Player::Grounded(void) {

	bool ground_hit=false;

	b2Vec2 player_position=phys_object->GetPosition();
	b2Vec2 player_velocity=phys_object->GetLinearVelocity();
	b2Vec2 player_dimension=b2Vec2(0.5f,0.9f);

	int contact_count=GLPT_physics->GetWorld()->GetContactCount();
	b2Contact* contact_buffer=GLPT_physics->GetWorld()->GetContactList();

	if (player_velocity.y) return false;

	while(contact_buffer) {
		if (contact_buffer->GetFixtureA()->GetUserData()!=this && contact_buffer->GetFixtureB()->GetUserData()!=this && contact_buffer->IsTouching()) {
			contact_buffer=contact_buffer->GetNext();
			continue;
		}

		b2WorldManifold contact_mani;

		ground_hit=true;

		contact_buffer->GetWorldManifold(&contact_mani);
		int point_count=contact_buffer->GetManifold()->pointCount;

		for (int f=0;f<point_count;f++) {
			ground_hit &= (contact_mani.points[f].y <= player_position.y - player_dimension.y);
		}

		contact_buffer=contact_buffer->GetNext();

		if (ground_hit) {
			return true;
		}
	}

	return false;
}
