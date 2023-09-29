#include "DXTKFont.h"
#include <tchar.h>

#define SafeDelete(x) { delete x; x = 0; }
#define SAFE_RELEASE(x) { if(x){ x->Release(); x = 0; } }	// ���� ��Ÿ��

//#include <DirectXMath.h> // XMFLOAT
#include "SimpleMath.h"
#include "YunuMath.h"

DXTKFont::DXTKFont()
	: m_pSpriteBatch(nullptr), m_pSpriteFont(m_pSpriteFont), m_RasterizerState(nullptr), m_DepthStencilState(nullptr)
{

}

DXTKFont::~DXTKFont()
{
	SafeDelete(m_pSpriteFont);
	SafeDelete(m_pSpriteBatch);
}

void DXTKFont::Create(ID3D11Device* pDevice, ID3D11RasterizerState* rs, ID3D11DepthStencilState* ds)
{
	ID3D11DeviceContext* pDC = nullptr;
	pDevice->GetImmediateContext(&pDC);

	m_pSpriteBatch = new DirectX::SpriteBatch(pDC);

	TCHAR* _fileName = (TCHAR*)L"Font/gulim9k.spritefont";

	m_pSpriteFont = new DirectX::SpriteFont(pDevice, _fileName);
	m_pSpriteFont->SetLineSpacing(14.0f);
	//m_pSpriteFont->SetDefaultCharacter(' ');

	SAFE_RELEASE(pDC);

	m_RasterizerState = rs;
	m_DepthStencilState = ds;
}

void DXTKFont::DrawTest()
{
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, L"Hello, world! �ѱ��� �ȵǳ�? �c�氢�� ���� �ݶ� �z�z", DirectX::XMFLOAT2(10.f, 10.f));
	m_pSpriteBatch->End();
}

void DXTKFont::DrawTextColor(int x, int y, DirectX::XMFLOAT4 color, TCHAR* text, ...)
{
	TCHAR _buffer[1024] = L"";
	va_list vl;
	va_start(vl, text);
	_vstprintf(_buffer, 1024, text, vl);
	va_end(vl);

	// SpriteBatch�� ���� ���� ������Ʈ�� ������ �� �ִ�.
	// ������, �׳� Begin�� �ϸ� �������Ľǹ��� �ɼ��� D3D11_DEPTH_WRITE_MASK_ZERO�� �Ǵ� ��. DSS�� �ٷ��� �ʴ� ���ݿ��� ������ �� �� �ִ�.
	// �Ʒ�ó�� ���⿡ �ɼ��� ALL�� �־��༭ ZERO�� �Ǵ� ���� ���� ���� �ְ�, �ٸ� �� ������Ʈ�� �׸� �� ����������Ʈ�� ���� �� �� ���� �ִ�.
	// DX12���� ����������Ʈ���� �׷��� �������� �͵� ���ش� ����. ���� ���� �ȵ������..
	///m_pSpriteBatch->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, m_DepthStencilState/*, m_RasterizerState*/);
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, _buffer, DirectX::XMFLOAT2((float)x, (float)y), DirectX::SimpleMath::Vector4(color));
	m_pSpriteBatch->End();
}

