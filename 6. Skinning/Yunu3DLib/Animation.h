#pragma once
#include <vector>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;
///--------------------------------------------------
/// Animation Data�� �����ϱ� ���� Ŭ����
///--------------------------------------------------

class CAnimation_pos
{
public:
    int m_time;
    Vector3	m_pos;
};

class CAnimation_rot
{
public:
    int				m_time;
    Vector3		m_rot;
    float			m_angle;
    Quaternion	m_rotQT_accumulation;		// ��뺯���̱� ������ ���� ����� �����ؼ� ������ �ִ´� (�� ���ݱ����� ���뷮���� ��ȯ�Ѵ�)
};

class CAnimation_scl {

public:
    int				m_time;
    Vector3		m_scale;
    Vector3		m_scaleaxis;
    float			m_scaleaxisang;
    Quaternion	m_scalerotQT_accumulation;
};


///--------------------------------------------------
/// �� Ŭ������ ������ Animation Ŭ����
///--------------------------------------------------
class Animation {

public:

    // �ִϸ��̼� �������� �̸�. ������ ���������̴�
    std::string						        m_nodename;
    std::vector<CAnimation_pos>		m_position;
    std::vector<CAnimation_rot>		m_rotation;
    std::vector<CAnimation_scl>		m_scale;

    int		m_ticksperFrame;	// �� �������� ƽ ũ��. �̰��� AnimationTable�� ������ ������ ������Ʈ���� �ٸ� �� �ִ�.
    int		m_minKeyframe;		// Ű �����Ӱ��� �ּ�ũ��. �̰��� ���ǰͰ� �� ��ġ������ �ʴ´�.
};