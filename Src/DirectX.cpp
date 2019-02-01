#include "stdafx.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

namespace GFXSystem
{
	struct Texture
	{
		ID3D11Texture2D*			m_pTexture2D;
		ID3D11ShaderResourceView*   m_textureRV;
		D3D11_TEXTURE2D_DESC		m_2DDesc;
	};

	D3D_DRIVER_TYPE				g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL			g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*				g_pd3dDevice = NULL;
	ID3D11DeviceContext*		g_pImmediateContext = NULL;
	IDXGISwapChain*				g_pSwapChain = NULL;
	ID3D11RenderTargetView*		g_pRenderTargetView = NULL;
	ID3D11VertexShader*			g_pVertexShader = NULL;
	ID3D11PixelShader*			g_pPixelShader = NULL;
	ID3D11InputLayout*			g_pVertexLayout = NULL;
	ID3D11Buffer*				g_pVertexBuffer = NULL;
	ID3D11SamplerState*			g_framebufferSamplerState;
	std::vector<Texture*>		g_framebufferTextures;

	static Texture* s_pLockedTexture = nullptr;
	static Texture* s_pCurrentTexture = nullptr;

	//--------------------------------------------------------------------------------------
	// Helper for compiling shaders with D3DX11
	//--------------------------------------------------------------------------------------
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	#if defined( DEBUG ) || defined( _DEBUG )
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;
	#endif

		ID3DBlob* pErrorBlob;
		hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
			dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
		if( FAILED(hr) )
		{
			if( pErrorBlob != NULL )
				OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			if( pErrorBlob ) pErrorBlob->Release();
			return hr;
		}
		if( pErrorBlob ) pErrorBlob->Release();

		return S_OK;
	}

	//-------------------------------------------------------------------------------------------------

	static Texture* FindOrAllocateTexture( u32 width, u32 height ) 
	{
		for ( size_t i = 0; i < g_framebufferTextures.size(); i++ )
		{
			if ( g_framebufferTextures[i]->m_2DDesc.Width != width )
				continue;
			if ( g_framebufferTextures[i]->m_2DDesc.Height != height )
				continue;
			return g_framebufferTextures[i];
		}

		Texture* pTexture = new Texture();

		D3D11_TEXTURE2D_DESC desc;
		desc.Width				= width;
		desc.Height				= height;
		desc.MipLevels			= 1;
		desc.ArraySize			= 1;
		desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count	= 1;
		desc.SampleDesc.Quality	= 0;
		desc.Usage				= D3D11_USAGE_DYNAMIC;
		desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags			= 0;

		g_pd3dDevice->CreateTexture2D( &desc, NULL, &pTexture->m_pTexture2D );

		D3D11_SHADER_RESOURCE_VIEW_DESC		descSRV;
		ZeroMemory( &descSRV, sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ) );
		descSRV.Format = desc.Format;	
		descSRV.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
		descSRV.Texture2D.MipLevels			= 1;
		descSRV.Texture2D.MostDetailedMip	= 0;

		g_pd3dDevice->CreateShaderResourceView( pTexture->m_pTexture2D, &descSRV, &pTexture->m_textureRV );

		pTexture->m_2DDesc = desc;
		g_framebufferTextures.push_back( pTexture );
		return pTexture;
	}

	//-------------------------------------------------------------------------------------------------

	FrameBufferInfo LockFrameBuffer( u32 width, u32 height )
	{
		assert( s_pLockedTexture == nullptr ); // need to unlock currently locked texture before locking another one

		s_pLockedTexture = FindOrAllocateTexture( width, height );
		D3D11_MAPPED_SUBRESOURCE mappedTex;

		g_pImmediateContext->Map( s_pLockedTexture->m_pTexture2D, D3D11CalcSubresource( 0, 0, 1 ), D3D11_MAP_WRITE_DISCARD, 0, &mappedTex );

		FrameBufferInfo info;

		info.m_pData = (u32*)mappedTex.pData;
		info.m_pitch = mappedTex.RowPitch/4;
		info.m_width = width;
		info.m_height = height;
		info.m_pixelStride = 4;
		s_pCurrentTexture = s_pLockedTexture;
		return info;
	}

	//-------------------------------------------------------------------------------------------------

	void UnlockFrameBuffer( )
	{
		assert( s_pLockedTexture != nullptr ); // need to unlock currently locked texture before locking another one

		g_pImmediateContext->Unmap( s_pLockedTexture->m_pTexture2D, D3D11CalcSubresource( 0, 0, 1 ) );

		s_pLockedTexture = nullptr;
	}

	//--------------------------------------------------------------------------------------
	// Create Direct3D device and swap chain
	//--------------------------------------------------------------------------------------
	HRESULT Init( HWND hWnd )
	{
		 HRESULT hr = S_OK;

		RECT rc;
		GetClientRect( hWnd, &rc );
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;

		UINT createDeviceFlags = 0;
	#ifdef _DEBUG
	   // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE( driverTypes );

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE( featureLevels );

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory( &sd, sizeof( sd ) );
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
		{
			g_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
												D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
			if( SUCCEEDED( hr ) )
				break;
		}
		if( FAILED( hr ) )
			return hr;

		// Create a render target view
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
		if( FAILED( hr ) )
			return hr;

		hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
		pBackBuffer->Release();
		if( FAILED( hr ) )
			return hr;

		g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL );

		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		g_pImmediateContext->RSSetViewports( 1, &vp );

		// Compile the vertex shader
		ID3DBlob* pVSBlob = NULL;
		hr = CompileShaderFromFile( L"src\\Display.fx", "VS", "vs_4_0", &pVSBlob );
		if( FAILED( hr ) )
		{
			MessageBox( NULL,
						L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
			return hr;
		}

		// Create the vertex shader
		hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
		if( FAILED( hr ) )
		{	
			pVSBlob->Release();
			return hr;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE( layout );

		// Create the input layout
		hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
											  pVSBlob->GetBufferSize(), &g_pVertexLayout );
		pVSBlob->Release();
		if( FAILED( hr ) )
			return hr;

		// Set the input layout
		g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

		// Compile the pixel shader
		ID3DBlob* pPSBlob = NULL;
		hr = CompileShaderFromFile( L"src\\Display.fx", "PS", "ps_4_0", &pPSBlob );
		if( FAILED( hr ) )
		{
			MessageBox( NULL,
						L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
			return hr;
		}

		// Create the pixel shader
		hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
		pPSBlob->Release();
		if( FAILED( hr ) )
			return hr;

		// Create vertex buffer
		float vertices[] =
		{
			 -1.0f,  1.0f, 0.5f , 0.0, 0.0f,
			  1.0f,  1.0f, 0.5f , 1.0, 0.0f,
			 -1.0f, -1.0f, 0.5f , 0.0, 1.0f,

			  1.0f,  1.0f, 0.5f , 1.0f, 0.0f,
			  1.0f, -1.0f, 0.5f , 1.0f, 1.0f,
			 -1.0f, -1.0f, 0.5f , 0.0f, 1.0f,
		};
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( float) * 5 * 6 ;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory( &InitData, sizeof(InitData) );
		InitData.pSysMem = vertices;
		hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
		if( FAILED( hr ) )
			return hr;

		// Set vertex buffer
		UINT stride = sizeof( float ) * 5;
		UINT offset = 0;
		g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

		// Set primitive topology
		g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		D3D11_SAMPLER_DESC samplerDesc;
		// Create a texture sampler state description.
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		HRESULT result = g_pd3dDevice->CreateSamplerState(&samplerDesc, &g_framebufferSamplerState);

		return S_OK;

	}
	//--------------------------------------------------------------------------------------
	// Render the frame
	//--------------------------------------------------------------------------------------
	void Render()
	{
		// Clear the back buffer 
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
		g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

		// Render a triangle
		g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
		g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
		g_pImmediateContext->PSSetSamplers(0, 1, &g_framebufferSamplerState);

		// Set shader texture resource in the pixel shader.
		if ( s_pCurrentTexture != nullptr )
		{
			g_pImmediateContext->PSSetShaderResources(0, 1, &s_pCurrentTexture->m_textureRV);
			g_pImmediateContext->Draw( 6, 0 );
		}

		// Present the information rendered to the back buffer to the front buffer (the screen)
		g_pSwapChain->Present( 0, 0 );
	}


	//--------------------------------------------------------------------------------------
	// Clean up the objects we've created
	//--------------------------------------------------------------------------------------
	void Shutdown()
	{
		if ( g_pImmediateContext ) g_pImmediateContext->ClearState();

		if ( g_pVertexBuffer ) g_pVertexBuffer->Release();
		if ( g_pVertexLayout ) g_pVertexLayout->Release();
		if ( g_pVertexShader ) g_pVertexShader->Release();
		if ( g_pPixelShader ) g_pPixelShader->Release();
		if ( g_pRenderTargetView ) g_pRenderTargetView->Release();
		if ( g_pSwapChain ) g_pSwapChain->Release();
		if ( g_pImmediateContext ) g_pImmediateContext->Release();
		if ( g_pd3dDevice ) g_pd3dDevice->Release();
		for ( size_t i = 0; i < g_framebufferTextures.size(); i++ )
		{
			g_framebufferTextures[i]->m_pTexture2D->Release();
			g_framebufferTextures[i]->m_textureRV->Release();
		}
	}
}

//-------------------------------------------------------------------------------------------------
