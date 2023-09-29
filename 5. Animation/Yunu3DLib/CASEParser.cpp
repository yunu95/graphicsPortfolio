#include "CASEParser.h"


CASEParser::CASEParser()
{
    m_materialcount = 0;
    m_parsingmode = eNone;
}

CASEParser::~CASEParser()
{
    delete m_lexer;
}

bool CASEParser::Init()
{
    m_lexer = new ASE::CASELexer;

    return TRUE;
}
//---------------------------------------------------------------------------------------------------
// �ε��Ѵ�.
// �̰��� �����ٸ� ������ ���������� ��� �����͸� �о �� �־�� �Ѵ�.
//
// Parsing�� ����:
// �׻� ������ �������� parsing�̶�� ���� �����Ͱ� ���� �԰ݿ� �°� �� �ִٴ� ���� ������ �Ѵ�.
// ������, ���� ���ο� ������ �ٲ�� �� �ִ°��� �����ϴٴ��� �ϴ� ��Ģ�� �����
// �˻��ϴ� ��ƾ�� ���������� ����. �ϴ��� ������� ���� �Ծ����� �� �ִٴ°��� ��������.
// -> �׷���, ��� ȣ���� �ϴ� �ϳ��� ū �Լ��� �ذ�ô�.
// -> depth�� �������� �����ۿ� ���� ���� �ڵ带 �־�߰ڴ�
//---------------------------------------------------------------------------------------------------
bool CASEParser::Load(LPSTR p_File)
{
    /// 0) ������ �ε��Ѵ�.
    if (!m_lexer->Open(p_File))
    {
        TRACE("������ ���� �߿� ������ �߻��߽��ϴ�!");
        return FALSE;
    }

    /// 1) �ٷ��̾ƺ����豳���� �Ŀ�Ǯ�� ����Լ� �Ѱ��� ó�� ��!
    Parsing_DivergeRecursiveALL(0);

    return TRUE;
}

/// 1�� ��ȯ

// CScenedata ����
bool CASEParser::TranslateToD3DFormat_scene(Scenedata* pscene)
{
    // �� ����
    *pscene = this->m_scenedata;
    return TRUE;
}

/// <summary>
/// �޽��� ����ȭ�� �� �ش�.
/// �븻��, �ؽ��� ��ǥ�� ���� ���ؽ��� �ø���, ��ø�Ǵ°��� �����ϰ�..
/// </summary>
bool CASEParser::Convert_Optimize(Mesh* pMesh)
{
    // 2021.04.12
    // ���� ���� (���ÿ�)

    return FALSE;
}

bool CASEParser::ConvertAll(Mesh* pMesh)
{
    // ���ؽ����� �ϴ� ��� ����
    for (unsigned int i = 0; i < pMesh->m_meshvertex.size(); i++)
    {
        Vertex* _pVertex = new Vertex;
        _pVertex->m_pos = pMesh->m_meshvertex[i]->m_pos;

        pMesh->m_opt_vertex.push_back(_pVertex);
    }

    // �������� face�� �������� �� �ε����� ã�ƾ� ��
    unsigned int _faceCount = pMesh->m_meshface.size();
    TRACE("FaceCount : %d", _faceCount);

    for (unsigned int i = 0; i < pMesh->m_meshface.size(); i++)
    {
        TRACE("FaceNum : %d / %d\n", i, _faceCount);

        for (int j = 0; j < 3; j++)
        {
            Vertex* _nowVertex = pMesh->m_opt_vertex[pMesh->m_meshface[i]->m_vertexindex[j]];

            // face�������� ���ؽ��� �븻�� �־��ش�.
            // (���ÿ�)
            _nowVertex->m_normal.x = 0;
            _nowVertex->m_normal.y = 0;
            _nowVertex->m_normal.z = 0;
        }
    }

    // �ε����� �׳� ����
    pMesh->m_opt_index = new IndexList[pMesh->m_meshface.size()];

    for (unsigned int i = 0; i < pMesh->m_meshface.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            pMesh->m_opt_index[i].index[j] = pMesh->m_meshface[i]->m_vertexindex[j];
        }
    }

    return FALSE;
}

ASEParser::Mesh* CASEParser::GetMesh(int index)
{
    return m_MeshList[index];
}

//----------------------------------------------------------------
// ��� ȣ����� ������ �ϴ� �б� �Լ��̴�.
//
// �ϴ� �а�, ��ū�� ���� �����Ѵ�.
//
// ��͸� ���� ������ { �� ���� ����̴�.
// �����ϴ� ������ '}'�� �����ų� TOKEND_END�� ������ ���̴�.
//
// ���Ҿ�, ��͸� ���� �� ��ū�� �о�, ���� �� �Լ��� � ��������� �����Ѵ�.
//
//----------------------------------------------------------------
void CASEParser::Parsing_DivergeRecursiveALL(int depth)
{
    //----------------------------------------------------------------------
    // ���������� ����
    // ��� �����ٰ� ���� �̻� ������ ���� ���(����)�� üũ�ϱ� ���� ����
    int i = 0;
    // ����Ʈ�� �����͸� �־�� �� �� ���̴� ����Ʈ ī����.
    int listcount = 0;		// �ſ� C����� �ٸ� �ذ�å�� ������?
    // �̰��� �� ��ū�� � �����ΰ��� �Ǻ����ش�.
    // �̰��� ��� ������ �ϰ� ��͸� �����°��� �����ϴ�? -> �߸��� ��͹���̴�?
    LONG nowtoken;
    //----------------------------------------------------------------------

    Matrix TM1, TM2;
    Quaternion tempQT;
    Quaternion prevQT;
    Quaternion resultQT;

    /// �̰��� �ϸ� �� ���� ��ū�� �а�, �� ������ ������ �� �� �ִ�.
    while (nowtoken = m_lexer->GetToken(m_TokenString), nowtoken != TOKEND_BLOCK_END)
    {
        // �ϴ� �� ���� ��ū�� �а�, �װ��� ��ȣ �ݱⰡ �ƴ϶��.
        // �Ѿ�� ��ū�� ���� ó�����ش�.

        static int iv = 0;

        switch (nowtoken)
        {
        case TOKEND_BLOCK_START:

            Parsing_DivergeRecursiveALL(depth++);
            break;

        case TOKENR_HELPER_CLASS:
            break;

            //--------------------
            // 3DSMAX_ASCIIEXPORT
            //--------------------

        case TOKENR_3DSMAX_ASCIIEXPORT:
            m_data_asciiexport = Parsing_NumberLong();
            break;

            //--------------------
            // COMMENT
            //--------------------

        case TOKENR_COMMENT:
            Parsing_String();	// �׳� m_TokenString�� �о������ ���� ��.
            //AfxMessageBox( m_TokenString, NULL, NULL);		/// �ӽ÷� �ڸ�Ʈ�� ����غ���
            break;

            //--------------------
            // SCENE
            //--------------------

        case TOKENR_SCENE:
            //
            break;
        case TOKENR_SCENE_FILENAME:
            m_scenedata.m_filename = Parsing_String();		// �ϰ��� �ִ� �Լ��� ����� ���� String�� Int�� ��������.
            break;
        case TOKENR_SCENE_FIRSTFRAME:
            m_scenedata.m_firstframe = Parsing_NumberLong();
            break;
        case TOKENR_SCENE_LASTFRAME:
            m_scenedata.m_lastframe = Parsing_NumberLong();
            break;
        case TOKENR_SCENE_FRAMESPEED:
            m_scenedata.m_framespeed = Parsing_NumberLong();
            break;
        case TOKENR_SCENE_TICKSPERFRAME:
            m_scenedata.m_ticksperframe = Parsing_NumberLong();
            break;
        case TOKENR_SCENE_MESHFRAMESTEP:
            m_scenedata.m_meshframestep = Parsing_NumberLong();
            break;
        case TOKENR_SCENE_KEYFRAMESTEP:
            m_scenedata.m_keyframestep = Parsing_NumberLong();
            break;
        case TOKENR_SCENE_BACKGROUND_STATIC:
            m_scenedata.m_scene_background_static.x = Parsing_NumberFloat();
            m_scenedata.m_scene_background_static.y = Parsing_NumberFloat();
            m_scenedata.m_scene_background_static.z = Parsing_NumberFloat();
            break;
        case TOKENR_SCENE_AMBIENT_STATIC:
            m_scenedata.m_scene_ambient_static.x = Parsing_NumberFloat();
            m_scenedata.m_scene_ambient_static.y = Parsing_NumberFloat();
            m_scenedata.m_scene_ambient_static.z = Parsing_NumberFloat();
            break;

        case TOKENR_SCENE_ENVMAP:
        {
            // �׳� ���� ������ �о������ }�� ���ö�����
            while (nowtoken = m_lexer->GetToken(m_TokenString), nowtoken != TOKEND_BLOCK_END) {
            }
        }
        break;

        //--------------------
        // MATERIAL_LIST
        //--------------------


        //--------------------
        // GEOMOBJECT
        //--------------------

        case TOKENR_GROUP:
            //	�� ���� �׷� ����. �� ������ �̸��� ��Ʈ������ ������� �ϴµ�.
            break;

        case TOKENR_HELPEROBJECT:
            // �ϴ� �����ϰ�
            // ������Ʈ�� Ÿ�� ������. �̰Ϳ� ���� ���� �ٸ� �Ľ� ��� �ߵ�.
            break;

        case TOKENR_GEOMOBJECT:
            /// �� ��ū�� �����ٴ°� ���ο� �޽ð� ����ٴ� ���̴�. ���� ������ mesh�� �ϳ� ����, �� �����͸� ����Ʈ�� �ְ�, m_onemesh�� �� �����͸� ����, �״�� ���� �ɱ�?
            break;

        case TOKENR_NODE_NAME:
            // ��� ������ ������Ʈ���� ���� �� �� �ִ� ������ ���̴�.
            // ��忡 ���� �־�� �� ���� �ٸ���.
            break;

        case TOKENR_NODE_PARENT:
            // �� ����� �θ� ����� ����.
            // �ϴ� �Է��� �ϰ�, ���߿� ��������.
            break;

            /// NODE_TM

        case TOKENR_NODE_TM:
            //m_parsingmode	=	eGeomobject;
            // (NODE_TM���� ���� �� NODE_NAME�� �ѹ� �� ���´�.)
            // (Animation���� ������ �ؾ� �ϱ� ������ �̷��� ��带 �����ش�)

            /// �Դٰ�,
            // ī�޶�� NodeTM�� �ι� ���´�. �ι�°��� ���� �ʴ´�.
            // �ƿ� �� ��Ϳ��� ������ ��Ű�� ������. �߰��� �д°��� �����ؾ� �ϱ� ������...
            //if (m_onemesh->m_camera_isloadTarget) {
            //	'}'�� ���ö����� �����°� ������ ����! �� �̷��� �ȵǳ�..
            // ��ͱ����� �������̴�....

            break;

        case TOKENR_INHERIT_POS:
            // ī�޶�� NodeTM�� �ι� ���´�. �ι�°��� ���� �ʴ´�.
            break;
        case TOKENR_INHERIT_ROT:
            break;
        case TOKENR_INHERIT_SCL:
            break;
        case TOKENR_TM_ROW0:
            break;
        case TOKENR_TM_ROW1:
            break;
        case TOKENR_TM_ROW2:
            break;
        case TOKENR_TM_ROW3:
            break;
        case TOKENR_TM_POS:
            break;
        case TOKENR_TM_ROTAXIS:
            break;
        case TOKENR_TM_ROTANGLE:
            break;
        case TOKENR_TM_SCALE:
            break;
        case TOKENR_TM_SCALEAXIS:
            break;
        case TOKENR_TM_SCALEAXISANG:
            // ���� ī�޶� ���¿��ٸ� �̹� ��带 ���� ������ ǥ�����ش�.
            break;


            /// MESH

        case TOKENR_MESH:
            //
            Create_onemesh_to_list();
            break;
        case TOKENR_TIMEVALUE:
            break;
        case TOKENR_MESH_NUMBONE:
            // �̰� �ִٸ� �̰��� Skinned Mesh��� ������ ���´�.
            // ���� �Է�
            break;
        case TOKENR_MESH_NUMSKINWEIGHT:
            break;
        case TOKENR_MESH_NUMVERTEX:
            m_OneMesh->m_mesh_numvertex = Parsing_NumberInt();
            m_OneMesh->m_meshvertex.resize(m_OneMesh->m_mesh_numvertex);
            break;
        case TOKENR_MESH_NUMFACES:
            m_OneMesh->m_mesh_numfaces = Parsing_NumberInt();
            m_OneMesh->m_meshface.resize(m_OneMesh->m_mesh_numfaces);
            m_OneMesh->m_opt_index = new IndexList[m_OneMesh->m_mesh_numfaces];
            break;

            /// MESH_VERTEX_LIST

        case TOKENR_MESH_VERTEX_LIST:
            //
            // ���ؽ��� ������ ����־�� �ϴµ�
            // �̹� ���ͷ� ������ �� �����Ƿ� �׳� ������ �ȴ�.
            break;
        case TOKENR_MESH_VERTEX:
        {
            // ������ �Է�
            auto vertex = new Vertex();
            int index = Parsing_NumberInt();
            vertex->m_pos.x = Parsing_NumberFloat();
            vertex->m_pos.y = Parsing_NumberFloat();
            vertex->m_pos.z = Parsing_NumberFloat();
            m_OneMesh->m_meshvertex[index] = vertex;
        }
        break;

        /// Bone

        case TOKENR_SKIN_INITTM:
            break;
        case TOKENR_BONE_LIST:
            break;
        case TOKENR_BONE:
        {
            /// ��� ü���� �� �ְ�, Bone�� �����ϰ� �ִٴ� ���� �̰��� ��Ű�� ������Ʈ��� ���̴�.
            // �� �ϳ��� ���� �ӽ� ������ ����, ���Ϳ� �ְ�
            // Bone�� �ѹ��� �о� ����
        }
        break;
        //�� ������ ���� �̸��� �־�� �Ѵ�. ������ {�� �� �� �� �������Ƿ� �ӽ� ������ �����μ� �����ؾ߰���.
        case TOKENR_BONE_NAME:
        case TOKENR_BONE_PROPERTY:
            // �� ���� ABSOLUTE�� ������� �ϴµ�, �� ���� ����.
            break;
            // �������� TM_ROW0~3�� �����µ� ���� ���õ�..

        case TOKENR_MESH_WVERTEXS:
            break;

        case TOKENR_MESH_WEIGHT:
        {
            // ���ؽ� �ϳ��� ������ ���� ����Ʈ�� ����
        }
        break;
        case TOKENR_BONE_BLENGING_WEIGHT:
        {
            // ��ü �� �ܰ踦 ���°ž�...
            // ����ġ �Ѱ��� ���� ����Ʈ�� �ִ´�
            /// �� ��....
        }
        break;


        /// MESH_FACE_LIST
        case TOKENR_MESH_FACE_LIST:
            //
            break;
        case TOKENR_MESH_FACE:
        {
            // Face�� ��ȣ�ε�...

            // A:�� �а�
            // B:
            // C:
            int index = Parsing_NumberInt();
            m_OneMesh->m_meshface[index] = new Face();
            Parsing_String(); // A:
            m_OneMesh->m_meshface[index]->m_vertexindex[0] = Parsing_NumberInt();
            Parsing_String(); // B:
            m_OneMesh->m_meshface[index]->m_vertexindex[1] = Parsing_NumberInt();
            Parsing_String(); // C:
            m_OneMesh->m_meshface[index]->m_vertexindex[2] = Parsing_NumberInt();

            m_OneMesh->m_opt_index[index].index[0] = m_OneMesh->m_meshface[index]->m_vertexindex[0];
            m_OneMesh->m_opt_index[index].index[1] = m_OneMesh->m_meshface[index]->m_vertexindex[1];
            m_OneMesh->m_opt_index[index].index[2] = m_OneMesh->m_meshface[index]->m_vertexindex[2];

            /// (�ڿ� ������ �� ������ default�� ���� ��ŵ�� ���̴�.)
            /// ......

            // ���Ϳ� �־��ش�.
        }
        break;
        case TOKENR_MESH_FACENORMAL:
        {
            int index = Parsing_NumberInt();
            m_OneMesh->m_meshface[index]->m_normal.x = Parsing_NumberFloat();
            m_OneMesh->m_meshface[index]->m_normal.y = Parsing_NumberFloat();
            m_OneMesh->m_meshface[index]->m_normal.z = Parsing_NumberFloat();

            Parsing_String();
            Parsing_NumberInt();

            m_OneMesh->m_meshface[index]->m_normalvertex[0].x = Parsing_NumberFloat();
            m_OneMesh->m_meshface[index]->m_normalvertex[0].y = Parsing_NumberFloat();
            m_OneMesh->m_meshface[index]->m_normalvertex[0].z = Parsing_NumberFloat();

            Parsing_String();
            Parsing_NumberInt();

            m_OneMesh->m_meshface[index]->m_normalvertex[1].x = Parsing_NumberFloat();
            m_OneMesh->m_meshface[index]->m_normalvertex[1].y = Parsing_NumberFloat();
            m_OneMesh->m_meshface[index]->m_normalvertex[1].z = Parsing_NumberFloat();

            Parsing_String();
            Parsing_NumberInt();
            
            m_OneMesh->m_meshface[index]->m_normalvertex[2].x = Parsing_NumberFloat();
            m_OneMesh->m_meshface[index]->m_normalvertex[2].y = Parsing_NumberFloat();
            m_OneMesh->m_meshface[index]->m_normalvertex[2].z = Parsing_NumberFloat();
        }
        break;
        case TOKENR_MESH_VERTEXNORMAL:
        {
        }
        break;
        case TOKENR_MESH_NUMTVERTEX:
            break;

            /// MESH_TVERTLIST

        case TOKENR_MESH_TVERTLIST:
            //
            break;
        case TOKENR_MESH_TVERT:
        {
            // ���ؽ��� �ε����� �����µ� ������ ������ �����Ƿ� ������.
            // ���ο� TVertex�� ���� ���Ϳ� �ִ´�
        }
        break;
        case TOKENR_MESH_NUMTVFACES:
            break;


        case TOKEND_END:
            // �Ƹ��� �̰� ������ ���� ��Ÿ�������ΰ� ������. while�� Ż���ؾ� �ϴµ�?

            //AfxMessageBox("������ ���� �� �� �����ϴ�!", MB_OK, NULL);
            TRACE("TRACE: �Ľ���: ������ ���� �ý��ϴ�!\n");
            return;

            break;

            /// ���� �ƹ��͵� �ش����� ������
        default:
            // �ƹ��͵� ���� �ʴ´�.
            break;

        }	// switch()


        ///-----------------------------------------------
        /// ���� �ڵ�.
        i++;
        if (i > 1000000)
        {
            // ������ 1000000���̻��̳� �� ������ ����. (�����Ͱ� 100000���� �ƴ��̻�)
            // ���� 1000000�̻� ���Ҵٸ� Ȯ���� ���� ������ �ִ� ���̹Ƿ�
            TRACE("������ �鸸�� ���ҽ��ϴ�!");
            return;
        }
        /// ���� �ڵ�.
        ///-----------------------------------------------

    }		// while()

    // ������� �Դٸ� while()�� ����ٴ� ���̰� �� ����
    // ��ȣ�� �ݾҴٴ� ���̹Ƿ�
    // �������� (����Լ�)

    return;
}


///----------------------------------------------------------------------
/// parsing�� ���� ������ �Լ���
///----------------------------------------------------------------------

// long�� �о �������ش�.
int CASEParser::Parsing_NumberLong()
{
    LONG			token;
    LONG			tempNumber;

    token = m_lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
    tempNumber = strtoul(m_TokenString, NULL, 10);

    return			tempNumber;
}

// float
float CASEParser::Parsing_NumberFloat()
{
    LONG			token;
    float			tempNumber;

    token = m_lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
    tempNumber = (float)atof(m_TokenString);

    return			tempNumber;
}

// String
LPSTR CASEParser::Parsing_String()
{
    /// ��m_TokenString ( char[255] ) �̱� ������ CString�� ������ ���� ���Ŷ� �����ߴµ�, �������� CString�� �� ������� �� ����. �˾Ƽ� �޾Ƶ��̴µ�?
    m_lexer->GetToken(m_TokenString);

    return m_TokenString;
}


// int
int CASEParser::Parsing_NumberInt() {

    LONG			token;
    int				tempNumber;

    token = m_lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
    tempNumber = (int)atoi(m_TokenString);

    return			tempNumber;
}


// 3���� Float�� ���� �ϳ���
Vector3 CASEParser::Parsing_NumberVector3()
{
    LONG				token;
    Vector3			tempVector3;

    token = m_lexer->GetToken(m_TokenString);
    tempVector3.x = (float)atof(m_TokenString);
    token = m_lexer->GetToken(m_TokenString);
    tempVector3.z = (float)atof(m_TokenString);
    token = m_lexer->GetToken(m_TokenString);
    tempVector3.y = (float)atof(m_TokenString);

    return			tempVector3;		// ����ƽ ������ ���۷������ٴ� �� ������ ����.
}

///--------------------------------------------------
/// ���ο��� ������ ����, ����Ʈ�� �ִ´�
///--------------------------------------------------
// �޽ø� �ϳ� �������� �����ϰ�, �� �����͸� ����Ʈ�� �ִ´�.
void CASEParser::Create_onemesh_to_list()
{
    Mesh* temp = new Mesh;
    m_OneMesh = temp;
    m_OneMesh->m_scenedata = m_scenedata;		// Ŭ������ �� ����
    m_MeshList.push_back(m_OneMesh);
}

// ��Ʈ���� �ϳ��� �������� �����ϰ�, �� �����͸� ����Ʈ�� �ִ´�.
void CASEParser::Create_materialdata_to_list()
{
    ASEMaterial* temp = new ASEMaterial;
    m_materialdata = temp;
    m_list_materialdata.push_back(m_materialdata);
}

// �ִϸ��̼ǵ����� �ϳ��� �������� �����ϰ�, �� �����͸� ����Ʈ�� �ִ´�.
void CASEParser::Create_animationdata_to_list()
{
    Animation* temp = new Animation;
    m_animation = temp;
    m_list_animation.push_back(m_animation);
}

// ���� �ϳ���..
void CASEParser::Create_onevertex_to_list()
{
    Vertex* temp = new Vertex;
    m_OneMesh->m_meshvertex.push_back(temp);
}

















