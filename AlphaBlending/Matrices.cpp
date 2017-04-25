#include <Windows.h>
#include <Mmsystem.h>
#include <d3dx9.h>

LPDIRECT3D9 g_pD3D = nullptr;  // D3D 
LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;  // 렌더링하는 D3D 디바이스
LPDIRECT3DVERTEXBUFFER9 g_pVB = nullptr;    // 정점 버퍼
LPDIRECT3DTEXTURE9 g_pTexture1 = nullptr;    // 텍스처0(벽)
LPDIRECT3DTEXTURE9 g_pTexture2 = nullptr;
LPDIRECT3DTEXTURE9 g_pTexture3 = nullptr;
LPDIRECT3DTEXTURE9 g_pTexture4 = nullptr;
LPDIRECT3DTEXTURE9 g_pTexture5 = nullptr;// 텍스처1(Light Map)
LPDIRECT3DTEXTURE9 g_pTexture6 = nullptr;
LPDIRECT3DTEXTURE9 g_pTexture7 = nullptr;
LPDIRECT3DTEXTURE9 g_pTexture8 = nullptr;

											 // 사용자 정의 정점 구조체
struct CUSTOMVERTEX {
	D3DXVECTOR3 position;   // 정점 좌표
	D3DCOLOR color;         // 정점 색깔
	FLOAT tu, tv;           // 텍스처 좌표
};

// 사용자가 설정한 정점에 대한 정보를 나타내는 FVF 값
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2)

HRESULT InitD3D(HWND hWnd);
HRESULT InitGeometry();
HRESULT InitTexture();
VOID Cleanup();
VOID Render();
VOID SetupMatrices();
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// 윈도우 클래스 선언 및 초기화, 등록
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"D3D Tutorial", NULL };
	RegisterClassEx(&wc);

	// 윈도우 생성
	HWND hWnd = CreateWindow(L"D3D Tutorial", L"D3D Tutorial 01: CreateDevice", WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, wc.hInstance, NULL);

	// Direct3D 초기화
	if (SUCCEEDED(InitD3D(hWnd))) {
		// 정점 버퍼 초기화
		if (SUCCEEDED(InitGeometry())) {
			// 텍스처 초기화
			if (SUCCEEDED(InitTexture())) {
				// 윈도우 출력
				ShowWindow(hWnd, SW_SHOWDEFAULT);
				UpdateWindow(hWnd);

				// 메세지 루프
				MSG msg;
				ZeroMemory(&msg, sizeof(msg));

				while (msg.message != WM_QUIT) {
					// GetMessage() 함수의 달리 대기하지 않는다.
					if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					else {
						// 처리할 메세지가 없으면 렌더링한다.
						Render();
					}
				}
			}
		}
	}

	// 등록한 클래스의 메모리를 반환하고, 등록에서 제외한다.
	UnregisterClass(L"D3D Tutorial", wc.hInstance);

	return 0;
}

HRESULT InitD3D(HWND hWnd)
{
	// D3D 객체 생성
	if (nullptr == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		return E_FAIL;
	}

	// D3D 디바이스를 생성하기 위해서 필요한 구조체
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));  // 0으로 초기화하지 않으면 결과가 나오지 않을 수 있다.

	d3dpp.Windowed = TRUE;  // 창모드
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// D3DDEVTYPE_HAL은 하드웨어 가속을 지원하도록 설정한다. 
	// 전역변수 g_pd3dDevice에 D3D 디바이스 포인터를 저장한다. 
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
		return E_FAIL;
	}

	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	// 삼각형의 뒷면을 표시하기 위한 코드, 컬링 기능을 끈다.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// 조명 끄기
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// z 버퍼 기능을 켠다.
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	return S_OK;
}

// 정점 버퍼 초기화, 텍스처 초기화
HRESULT InitGeometry()
{
	CUSTOMVERTEX vertecies[] = {
		{ D3DXVECTOR3(-1.0f, 1.0f, 0.f), D3DCOLOR(0xffffffff), 0.f, 0.f },
		{ D3DXVECTOR3(1.0f, 1.0f, 0.f), D3DCOLOR(0xffffffff), 1.f, 0.f },
		{ D3DXVECTOR3(-1.0f, -1.0f, 0.f), D3DCOLOR(0xffffffff), 0.f, 1.f },

		{ D3DXVECTOR3(1.0f, 1.0f, 0.f), D3DCOLOR(0xffffffff), 1.f, 0.f },
		{ D3DXVECTOR3(-1.0f, -1.0f, 0.f), D3DCOLOR(0xffffffff), 0.f, 1.f },
		{ D3DXVECTOR3(1.0f, -1.0f, 0.f), D3DCOLOR(0xffffffff), 1.f, 1.f },
	};

	// 정점 버퍼 생성
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(sizeof(vertecies), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
		return E_FAIL;
	}

	CUSTOMVERTEX* pVertices;

	// Lock() 함수의 첫 번째와 두 번째 인자를 0으로 전달하면
	// Vertex Buffer의 전체 버퍼 크기로 설정
	if (FAILED(g_pVB->Lock(0, 0, (void**)&pVertices, 0))) {
		return E_FAIL;
	}
	memcpy(pVertices, &vertecies, sizeof(vertecies));

	g_pVB->Unlock();

	return S_OK;
}

HRESULT InitTexture()
{
	// 텍스처를 설정한다.
	D3DXCreateTextureFromFile(g_pd3dDevice, L"sprite1.bmp", &g_pTexture1);
	D3DXCreateTextureFromFile(g_pd3dDevice, L"sprite2.bmp", &g_pTexture2);
	D3DXCreateTextureFromFile(g_pd3dDevice, L"sprite3.bmp", &g_pTexture3);
	D3DXCreateTextureFromFile(g_pd3dDevice, L"sprite4.bmp", &g_pTexture4);
		

	D3DXCreateTextureFromFile(g_pd3dDevice, L"Road1.bmp", &g_pTexture5);
	D3DXCreateTextureFromFile(g_pd3dDevice, L"Road2.bmp", &g_pTexture6);
	D3DXCreateTextureFromFile(g_pd3dDevice, L"Road3.bmp", &g_pTexture7);
	D3DXCreateTextureFromFile(g_pd3dDevice, L"Road4.bmp", &g_pTexture8);
	

	return S_OK;
}

// 객체 리소스를 반환하는 함수
VOID Cleanup()
{
	// 객체의 리소스를 반환하는 순서가 중요하다.
	// 이 순서가 잘못되면 에러를 발생시킨다.
	// 생성의 역순

	if (g_pTexture1 != nullptr) {
		g_pTexture1->Release();
	}

	if (g_pTexture2 != nullptr) {
		g_pTexture2->Release();
	}
	if (g_pTexture3 != nullptr) {
		g_pTexture3->Release();
	}
	if (g_pTexture4 != nullptr) {
		g_pTexture4->Release();
	}
	if (g_pTexture5 != nullptr) {
		g_pTexture5->Release();
	}

	if (g_pTexture6 != nullptr) {
		g_pTexture6->Release();
	}
	if (g_pTexture7 != nullptr) {
		g_pTexture7->Release();
	}
	if (g_pTexture8 != nullptr) {
		g_pTexture8->Release();
	}

	if (g_pVB != nullptr) {
		g_pVB->Release();
	}

	if (g_pd3dDevice != nullptr) {
		g_pd3dDevice->Release();
	}

	if (g_pD3D != nullptr) {
		g_pD3D->Release();
	}
}

// 화면을 그리는 함수
VOID Render()
{
	static int counter = 0;
	
	if (nullptr == g_pD3D || nullptr == g_pd3dDevice || nullptr == g_pVB || nullptr == g_pTexture1 || nullptr == g_pTexture2 || nullptr == g_pTexture3 || nullptr == g_pTexture4 || nullptr == g_pTexture5) {
		return;
	}

	// 후면 버퍼를 파란색으로 채운다. z버퍼를 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	// 와이어 프레임으로 본다.
	//g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	// BeginScene() 함수로 디바이스에게 Rendering 시작을 알린다.
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {

		SetupMatrices();
		counter = counter + 1;
		switch (counter % 21)
		{
		case 5:
			g_pd3dDevice->SetTexture(0, g_pTexture1);
			break;
		case 10:
			g_pd3dDevice->SetTexture(0, g_pTexture2);
			break;
		case 15:
			g_pd3dDevice->SetTexture(0, g_pTexture3);
			break;
		case 20:
			g_pd3dDevice->SetTexture(0, g_pTexture4);
			break;

		}

		switch (counter % 21)
		{
		case 5:
			g_pd3dDevice->SetTexture(1, g_pTexture5);
			break;
		case 10:
			g_pd3dDevice->SetTexture(1, g_pTexture6);
			break;
		case 15:
			g_pd3dDevice->SetTexture(1, g_pTexture7);
			break;
		case 20:
			g_pd3dDevice->SetTexture(1, g_pTexture8);
			break;

		}
	
		// 생성한 텍스처에 스테이지를 할당한다.

		// 0번 텍스처에 0번 텍스처 인덱스를 사용한다.(FVF에 정점마다 8개의 텍스처 인덱스가 있다)
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		// 1번 텍스처에 0번 텍스처 인덱스를 사용한다.
		g_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);

		// 텍스처 맵핑을 할때 확대 필터를 사용하고, Linear 방식으로 보간한다.
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		g_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

		// D3DTSS_COLOROP, D3DTSS_ALPHAOP과 함께
		// D3DTOP_SELECTARG1를 사용하면 이번 stage에 input를 바로 output으로 전달한다.
		// 0번 스테이지의 컬러 값과 알파 값이 그대로 output으로 전달된다.
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

		// color operation을 modulate로 설정
		g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);

		g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		// 컬러의 첫 번째 인자는 현재 스테이지의 텍스처의 컬러 값
		g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);

		// 컬러의 두 번째 인자는 이전 스테이지에서 전달된 컬러 값
		g_pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

		// 알파의 첫 번째 인자는 현재 스테이지의 텍스처의 알파 값
		g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

		// 알파의 두 번째 인자는 이전 스테이지에서 전달된 알파 값
		g_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

		// 결국 SetTextureStageState() 함수를 사용하여 텍스처1과 텍스처2의 알파 블랜딩하였다.


		// 3번째 스테이지에서부터는 컬러와 알파값에 대한 연산을 하지 않는다.
		g_pd3dDevice->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		g_pd3dDevice->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


		// 정점 버퍼를 출력 스트림으로 설정한다.
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));

		// D3D에 정점 정보를 제공한다.
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

		// 실제 기하 물체를 그린다.
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

		// Rendering이 끝났음을 알린다.
		g_pd3dDevice->EndScene();
	}

	// 후면 버퍼를 현재 그리는 버퍼로 설정한다.
	// Double Buffering(이중 버퍼)와 관련된 내용이다.
	// 이중 버퍼를 사용하면서 이 함수를 실행하지 않으면 제대로 그려지지 않는다.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

// 월드, 뷰, 프로젝션 행렬 설정
VOID SetupMatrices()
{
	// 월드 행렬 설정
	D3DXMATRIXA16 matWorld;

	D3DXMatrixIdentity(&matWorld);
	// 아래줄의 주석을 해제하면 원통이 x축을 기준으로 회전한다.
	//D3DXMatrixRotationX( &matWorld, timeGetTime() / 500.0f );
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	// 뷰 행렬 설정
	D3DXVECTOR3 vEyePt(0.0f, 0.0f, -3.0f);  // 카메라의 위치
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);    // 카메라가 바라보는 지점
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);   // 카메라의 상향벡터

	D3DXMATRIXA16 matView;
	// 카메라 변환 행렬 계산
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// 계산된 카메라 변환 행렬을 적용
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// 프로젝션 행렬 설정
	D3DXMATRIXA16 matProj;
	// 투영 변환 행렬 계산
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.f);
	// 계산된 투영 변환 행렬을 적용
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

// 윈도우 프로시저 함수
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;

		// 메세지 루프 변경
		/*case WM_PAINT:
		Render();
		ValidateRect(hWnd, NULL);
		return 0;*/
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
