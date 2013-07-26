#include "Camera.h"

Camera* GLPT_camera;

Camera::Camera(void) {
	mode_2d=false;
	x=y=z=0.0f;
	lx=ly=0.0f;
	lz=-10.0f;
	fov=60.0f * 3.141f / 180.0f;
	dx=dy=dz=dxy=dxz=dyz=0.0f;
}

void Camera::SetPosition(float x,float y, float z) {
	this->x=x;
	this->y=y;
	this->z=z;
}

void Camera::GetPosition(float* x,float* y,float* z) {

	if (x) *x=this->x;
	if (y) *y=this->y;
	if (z) *z=this->z;

}

void Camera::SetFOV(float fov) {
	this->fov=fov * 3.141f / 180.0f;
}

void Camera::LookAt(float x, float y, float z) {
	lx=x;
	ly=y;
	lz=z;
}

void Camera::DrawPosition(float x, float y, float z) {
	dx=x;
	dy=y;
	dz=z;
}

void Camera::DrawAngle(float xy,float xz, float yz) {
	dxy=xy;
	dxz=xz;
	dyz=yz;
}

D3DXMATRIX Camera::GetTransform(void) {

	D3DXMATRIX translate,rotate,projection,view;
	D3DXMatrixTranslation(&translate,dx,dy,dz);
	D3DXMatrixRotationZ(&rotate,dxy);

	translate=rotate * translate;

	if (mode_2d) {
		D3DXMatrixOrthoLH(&projection,2.0f,2.0f,0.0f,10.0f);
		return (translate * projection);
	}

	D3DXMatrixLookAtLH(&view,&D3DXVECTOR3(x,y,z),&D3DXVECTOR3(lx,ly,lz),&D3DXVECTOR3(0,1,0));
	D3DXMatrixPerspectiveFovLH(&projection,fov,(float) GLPT_width / (float) GLPT_height,0.1f,1000.0f);

	return (translate * view * projection);
}