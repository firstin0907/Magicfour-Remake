#include <wrl.h>

class BitmapClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	BitmapClass(class D2DClass* direct2d, const wchar_t* filename);
	~BitmapClass() = default;
	
	inline float GetWidth() { return width_; }
	inline float GetHeight() { return height_; }

	inline struct ID2D1Bitmap* GetBitmap() { return bitmap_.Get(); }

private:
	float width_, height_;
	ComPtr<struct ID2D1Bitmap> bitmap_;
};