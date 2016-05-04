#pragma once

class MyQuadGeometry
{
public:
	MyQuadGeometry();
	~MyQuadGeometry();

	void SetPositionAttribute(unsigned int loc);
	void Build();
	void Render();

protected:
	int mPositionAttribute;

	unsigned int mVertexArray;
	unsigned int mVertexBuffer;
	unsigned int mIndexBuffer;
	void BuildGeometry();
};

