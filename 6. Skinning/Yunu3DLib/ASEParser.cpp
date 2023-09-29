#include "ASEParser.h"
#include <map>
#include <regex>
#include <algorithm>

ASEParser::ASEParser()
{
    m_materialcount = 0;
    m_parsingmode = eNone;
}

ASEParser::~ASEParser()
{
    delete m_lexer;
}

bool ASEParser::Init()
{
    m_lexer = new ASE::ASELexer;

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
bool ASEParser::Load(LPSTR p_File)
{
    ClearLoadedData();
    /// 0) ������ �ε��Ѵ�.
    if (!m_lexer->Open(p_File))
    {
        TRACE("������ ���� �߿� ������ �߻��߽��ϴ�!");
        return FALSE;
    }

    /// 1) �ٷ��̾ƺ����豳���� �Ŀ�Ǯ�� ����Լ� �Ѱ��� ó�� ��!
    Parsing_DivergeRecursiveALL(0, {});

    return TRUE;
}

/// 1�� ��ȯ

// CScenedata ����
bool ASEParser::TranslateToD3DFormat_scene(Scenedata* pscene)
{
    // �� ����
    *pscene = this->m_scenedata;
    return TRUE;
}

/// <summary>
/// �޽��� ����ȭ�� �� �ش�.
/// �븻��, �ؽ��� ��ǥ�� ���� ���ؽ��� �ø���, ��ø�Ǵ°��� �����ϰ�..
/// </summary>
bool ASEParser::Convert_Optimize(Mesh* pMesh)
{
    // 2021.04.12
    // ���� ���� (���ÿ�)

    return FALSE;
}

bool ASEParser::ConvertAll(Mesh* pMesh)
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

ASE::Mesh* ASEParser::GetMesh(int index)
{
    return m_MeshList[index].get();
}
vector<ASE::Mesh*> ASEParser::GetMeshes()
{
    vector<ASE::Mesh*> rawPtrs;
    std::transform(m_MeshList.begin(), m_MeshList.end(), back_inserter(rawPtrs),
        [](const std::unique_ptr<ASE::Mesh>& ptr) {return ptr.get(); });
    return rawPtrs;
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
void ASEParser::Parsing_DivergeRecursiveALL(int depth, vector<LONG> parents)
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
    LONG lastToken;
    //----------------------------------------------------------------------

    Matrix TM1, TM2;
    Quaternion tempQT;
    Quaternion prevQT;
    Quaternion resultQT;
    static int currentVertexIndex;
    static int currentBoneIndex;
    static int currentMaterialIndex;

    /// �̰��� �ϸ� �� ���� ��ū�� �а�, �� ������ ������ �� �� �ִ�.
    while (nowtoken = m_lexer->GetToken(m_TokenString), nowtoken != TOKEND_BLOCK_END)
    {
        // �ϴ� �� ���� ��ū�� �а�, �װ��� ��ȣ �ݱⰡ �ƴ϶��.
        // �Ѿ�� ��ū�� ���� ó�����ش�.

        static int iv = 0;

        switch (nowtoken)
        {
        case TOKEND_BLOCK_START:
        {
            vector<LONG> nextParents = parents;
            nextParents.push_back(lastToken);
            Parsing_DivergeRecursiveALL(depth++, nextParents);
            break;
        }
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
        case TOKENR_MESH_NUMTVERTEX:
        {
            int TVSize = Parsing_NumberInt();
            m_OneMesh->m_mesh_tvertex.resize(TVSize);
        }
        break;
        case TOKENR_MESH_MAPPINGCHANNEL:
        {
            Parsing_NumberInt();
        }
        break;
        case TOKENR_MESH_TVERT:
        {
            if (parents.size() > 2 && *(parents.end() - 2) == TOKENR_MESH_MAPPINGCHANNEL)
                break;

            int TVertexIndex = Parsing_NumberInt();
            m_OneMesh->m_mesh_tvertex[TVertexIndex] = new COneTVertex();
            m_OneMesh->m_mesh_tvertex[TVertexIndex]->m_u = Parsing_NumberFloat();
            m_OneMesh->m_mesh_tvertex[TVertexIndex]->m_v = Parsing_NumberFloat();
            m_OneMesh->m_mesh_tvertex[TVertexIndex]->m_w = Parsing_NumberFloat();
        }
        break;
        case TOKENR_MESH_TFACE:
        {
            int TVertexIndex = Parsing_NumberInt();
            if (m_OneMesh->m_meshface[TVertexIndex] == nullptr)
                m_OneMesh->m_meshface[TVertexIndex] = new ASE::Face;
            m_OneMesh->m_meshface[TVertexIndex]->m_TFace[0] = Parsing_NumberInt();
            m_OneMesh->m_meshface[TVertexIndex]->m_TFace[1] = Parsing_NumberInt();
            m_OneMesh->m_meshface[TVertexIndex]->m_TFace[2] = Parsing_NumberInt();
        }
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
        case TOKENR_MATERIAL_LIST:
        {
            break;
        }
        case TOKENR_MATERIAL_COUNT:
        {
            int count = Parsing_NumberInt();
            m_list_materialdata.clear();
            m_list_materialdata.resize(count);
            for (int i = 0; i < count; i++)
                m_list_materialdata[i] = make_unique<ASEMaterial>();

            break;
        }
        case TOKENR_MATERIAL:
        {
            currentMaterialIndex = Parsing_NumberInt();
            break;
        }
        case TOKENR_MATERIAL_AMBIENT:
        {
            m_list_materialdata[currentMaterialIndex]->m_material_ambient = Parsing_NumberVector3();
            break;
        }
        case TOKENR_MATERIAL_DIFFUSE:
        {
            m_list_materialdata[currentMaterialIndex]->m_material_diffuse = Parsing_NumberVector3();
            break;
        }
        case TOKENR_MATERIAL_SPECULAR:
        {
            m_list_materialdata[currentMaterialIndex]->m_material_specular = Parsing_NumberVector3();
            break;
        }
        case TOKENR_MAP_DIFFUSE:
        {
            m_list_materialdata[currentMaterialIndex]->m_map_diffuse = make_unique<MaterialMap>();
            break;
        }
        case TOKENR_BITMAP:
        {
            string* m_bitmap = nullptr;
            switch (*parents.rbegin())
            {
            case TOKENR_MAP_DIFFUSE:
                m_bitmap = &m_list_materialdata[currentMaterialIndex]->m_map_diffuse->m_bitmap;
                break;
            }
            assert(m_bitmap != nullptr, "parsing error : bitmap type is not specified");
            *m_bitmap = Parsing_String();
            regex pattern("(\\w*\.dds)");
            smatch matches;
            if (regex_search(*m_bitmap, matches, pattern))
                *m_bitmap = matches[0];
            break;
        }


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
        {
            string nodename = Parsing_String();
            if (*(parents.end() - 1) == TOKENR_GEOMOBJECT ||
                *(parents.end() - 1) == TOKENR_HELPEROBJECT)
            {
                if (boneIdxByNodeName.find(nodename) != boneIdxByNodeName.end())
                    nodeIdxByBoneIdx[boneIdxByNodeName[nodename]] = m_MeshList.size();
                Create_onemesh_to_list();
                m_OneMesh->m_nodename = nodename;
            }
            break;
        }

        case TOKENR_NODE_PARENT:
            m_OneMesh->m_nodeparent = Parsing_String();
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
            m_OneMesh->m_inherit_pos = Parsing_NumberVector3();
            break;
        case TOKENR_INHERIT_ROT:
            m_OneMesh->m_inherit_rot = Parsing_NumberVector3();
            break;
        case TOKENR_INHERIT_SCL:
            m_OneMesh->m_inherit_scl = Parsing_NumberVector3();
            break;
        case TOKENR_TM_ROW0:
            m_OneMesh->m_tm_row0 = Parsing_NumberVector3();
            break;
        case TOKENR_TM_ROW1:
            m_OneMesh->m_tm_row1 = Parsing_NumberVector3();
            break;
        case TOKENR_TM_ROW2:
            m_OneMesh->m_tm_row2 = Parsing_NumberVector3();
            break;
        case TOKENR_TM_ROW3:
            m_OneMesh->m_tm_row3 = Parsing_NumberVector3();
            break;
        case TOKENR_TM_POS:
            m_OneMesh->m_tm_pos = Parsing_NumberVector3();
            break;
        case TOKENR_TM_ROTAXIS:
            m_OneMesh->m_tm_rotaxis = Parsing_NumberVector3();
            break;
        case TOKENR_TM_ROTANGLE:
            m_OneMesh->m_tm_rotangle = Parsing_NumberFloat();
            break;
        case TOKENR_TM_SCALE:
            m_OneMesh->m_tm_scale = Parsing_NumberVector3();
            break;
        case TOKENR_TM_SCALEAXIS:
            m_OneMesh->m_tm_scaleaxis = Parsing_NumberVector3();
            break;
        case TOKENR_TM_SCALEAXISANG:
            // ���� ī�޶� ���¿��ٸ� �̹� ��带 ���� ������ ǥ�����ش�.
            break;


            /// MESH

        case TOKENR_MESH:
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
            //m_OneMesh->m_mesh_facenormal = new Vector3[m_OneMesh->m_mesh_numfaces];
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
            currentBoneIndex = Parsing_NumberInt();
            /// ��� ü���� �� �ְ�, Bone�� �����ϰ� �ִٴ� ���� �̰��� ��Ű�� ������Ʈ��� ���̴�.
            // �� �ϳ��� ���� �ӽ� ������ ����, ���Ϳ� �ְ�
            // Bone�� �ѹ��� �о� ����
        }
        break;
        //�� ������ ���� �̸��� �־�� �Ѵ�. ������ {�� �� �� �� �������Ƿ� �ӽ� ������ �����μ� �����ؾ߰���.
        case TOKENR_BONE_NAME:
        {
            boneIdxByNodeName[Parsing_String()] = currentBoneIndex;
        }
        case TOKENR_BONE_PROPERTY:
            // �� ���� ABSOLUTE�� ������� �ϴµ�, �� ���� ����.
            break;
            // �������� TM_ROW0~3�� �����µ� ���� ���õ�..

        case TOKENR_MESH_WVERTEXS:
            break;

        case TOKENR_MESH_WEIGHT:
        {
            // ���ؽ� �ϳ��� ������ ���� ����Ʈ�� ����
            currentVertexIndex = Parsing_NumberInt();
        }
        break;
        case TOKENR_BONE_BLENGING_WEIGHT:
        {
            int& blendIndex = m_OneMesh->m_meshvertex[currentVertexIndex]->m_blend_number;
            m_OneMesh->m_meshvertex[currentVertexIndex]->m_blend_indices[blendIndex] = Parsing_NumberInt();
            m_OneMesh->m_meshvertex[currentVertexIndex]->m_blend_weights[blendIndex] = Parsing_NumberFloat();
            blendIndex++;
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
            if (m_OneMesh->m_meshface[index] == nullptr)
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
            m_OneMesh->m_meshface[index]->m_normal = Parsing_NumberVector3();

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
        // vertex normal vectors are parsed above in case TOKENR_MESH_FACENORMAL:
        case TOKENR_MESH_VERTEXNORMAL:
        {
        }
        break;
        case TOKENR_MESH_TVERTLIST:
            //
            break;
        case TOKENR_MESH_NUMTVFACES:
            break;

        case TOKENR_TM_ANIMATION:
            Create_animationdata_to_list();
            m_animation->m_nodename = m_OneMesh->m_nodename;
            break;
        case TOKENR_CONTROL_POS_SAMPLE:
        {
            m_animation->m_position.push_back({ Parsing_NumberInt(),Parsing_NumberVector3() });
        }
        break;
        case TOKENR_CONTROL_ROT_TRACK:
        {
            m_lastQuaternionAcc = XMQuaternionIdentity();
        }
        break;
        case TOKENR_CONTROL_ROT_SAMPLE:
        {
            CAnimation_rot rot;
            rot.m_time = Parsing_NumberInt();
            rot.m_rot = Parsing_NumberVector3();
            rot.m_angle = Parsing_NumberFloat();
            //auto temp = rot.m_rot.z;
            //rot.m_rot.z = rot.m_rot.y;
            //rot.m_rot.y = temp;
            rot.m_angle *= -1;
            //rot.m_rotQT_accumulation = XMQuaternionMultiply(m_lastQuaternionAcc, Quaternion(XMQuaternionRotationAxis(rot.m_rot, rot.m_angle)));
            //rot.m_rotQT_accumulation = XMQuaternionMultiply(Quaternion(XMQuaternionRotationAxis(rot.m_rot, rot.m_angle)), m_lastQuaternionAcc);
            rot.m_rotQT_accumulation = XMQuaternionMultiply(m_lastQuaternionAcc, Quaternion(XMQuaternionRotationAxis(rot.m_rot, rot.m_angle)));
            //if (rot.m_time == 0)
            //{
                //rot.m_rotQT_accumulation = XMQuaternionMultiply(m_lastQuaternionAcc, Quaternion(XMQuaternionRotationAxis(rot.m_rot, rot.m_angle)));
            //}

            m_lastQuaternionAcc = rot.m_rotQT_accumulation;


            // come back for this for accumulation
            m_animation->m_rotation.push_back(rot);
        }
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


        lastToken = nowtoken;
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
int ASEParser::Parsing_NumberLong()
{
    LONG			token;
    LONG			tempNumber;

    token = m_lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
    tempNumber = strtoul(m_TokenString, NULL, 10);

    return			tempNumber;
}

// float
float ASEParser::Parsing_NumberFloat()
{
    LONG			token;
    float			tempNumber;

    token = m_lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
    tempNumber = (float)atof(m_TokenString);

    return			tempNumber;
}

// String
LPSTR ASEParser::Parsing_String()
{
    /// ��m_TokenString ( char[255] ) �̱� ������ CString�� ������ ���� ���Ŷ� �����ߴµ�, �������� CString�� �� ������� �� ����. �˾Ƽ� �޾Ƶ��̴µ�?
    m_lexer->GetToken(m_TokenString);

    return m_TokenString;
}


// int
int ASEParser::Parsing_NumberInt() {

    LONG			token;
    int				tempNumber;

    token = m_lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
    tempNumber = (int)atoi(m_TokenString);

    return			tempNumber;
}


// 3���� Float�� ���� �ϳ���
Vector3 ASEParser::Parsing_NumberVector3()
{
    LONG				token;
    Vector3			tempVector3;

    token = m_lexer->GetToken(m_TokenString);
    tempVector3.x = (float)atof(m_TokenString);
    token = m_lexer->GetToken(m_TokenString);
    tempVector3.y = (float)atof(m_TokenString);
    token = m_lexer->GetToken(m_TokenString);
    tempVector3.z = (float)atof(m_TokenString);

    return			tempVector3;		// ����ƽ ������ ���۷������ٴ� �� ������ ����.
}

///--------------------------------------------------
/// ���ο��� ������ ����, ����Ʈ�� �ִ´�
///--------------------------------------------------
// �޽ø� �ϳ� �������� �����ϰ�, �� �����͸� ����Ʈ�� �ִ´�.
void ASEParser::Create_onemesh_to_list()
{
    m_OneMesh = new Mesh();
    m_OneMesh->m_scenedata = m_scenedata;		// Ŭ������ �� ����
    m_MeshList.push_back(unique_ptr<Mesh>(m_OneMesh));
}

// ��Ʈ���� �ϳ��� �������� �����ϰ�, �� �����͸� ����Ʈ�� �ִ´�.
void ASEParser::Create_materialdata_to_list()
{
    ASEMaterial* temp = new ASEMaterial;
    m_materialdata = temp;
    m_list_materialdata.push_back(unique_ptr<ASEMaterial>(m_materialdata));
}

// �ִϸ��̼ǵ����� �ϳ��� �������� �����ϰ�, �� �����͸� ����Ʈ�� �ִ´�.
void ASEParser::Create_animationdata_to_list()
{
    m_animation = make_shared<Animation>();
    m_list_animation.push_back(m_animation);
}

// ���� �ϳ���..
void ASEParser::Create_onevertex_to_list()
{
    Vertex* temp = new Vertex;
    m_OneMesh->m_meshvertex.push_back(temp);
}
void ASEParser::ClearLoadedData()
{
    m_materialdata = nullptr;
    m_list_materialdata.clear();
    m_OneMesh = nullptr;
    m_MeshList.clear();
    nodeIdxByBoneIdx.clear();

    delete m_nowshapeline;
    m_nowshapeline = nullptr;

    delete m_nowshapevertex;
    m_nowshapevertex = nullptr;

    m_animation = nullptr;
    boneIdxByNodeName.clear();
    m_list_animation.clear();
}
