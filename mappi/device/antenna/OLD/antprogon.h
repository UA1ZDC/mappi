/**
   ����� ������������ ����� ���������� ��������� �������.
   �������� "��������", ���������� ������������ ������� � ���������� ���������,
   ������������ ����������� DSA � DSE. �������� ���������� ������� �������� � �������
   �������� �������. �� ���������� ������������ ����� ������� �������� ���������
   � �������� ����������� � ��� �� �������������.

   @author	������� �.�.
   @date	21.08.2009
*/
#ifndef MAPPI_DEVICE_PRBL_ANT_ANTPROGON_H
#define MAPPI_DEVICE_PRBL_ANT_ANTPROGON_H


namespace mappi {
  namespace receive {

    class AntProgon {

      float m_azimut;			// ���� �� �������
      float m_elevat;			// ���� �� ���� �����
      float m_azMin; //����������� ������
      float m_azMax; //������������ ������
      float m_elMin; //����������� ���� �����
      float m_elMax; //������������ ���� �����
      float m_DSA;			// ���������� �������� � ������� �� �������
      float m_DSE;			// ���������� �������� � ������� �� ���� �����
      float m_dsaMax;			// ������������ �������� ���������� �������� � ������� �� �������
      float m_dseMax;			// ������������ �������� ���������� �������� � ������� �� ���� �����
      
      // ��������� ����������� ����
      bool m_isPositivDirectAzimut;	// true �������������
      bool m_isPositivDirectElevat;	// true �������������, �.�. ����� � 180 ��� � 0
    
      /** ����������� ����������� ����. */
      void direction();
    
      AntProgon( const AntProgon& object );
      AntProgon& operator=( const AntProgon& object );
    
    public :
    
      AntProgon(float dsaMax, float dseMax, float azMin, float azMax, float elMin, float elMax);
      ~AntProgon();
    
      /**
	 ��������� ���������� ��������� �������, �������.
	 @return true - ���� �������� ���������.
      */
      void setStartPosition( const double& az, const double& el );
    
      /**
	 ��������� ���� ������� �� �������, �������.
	 ������������ �������� DSA = 9 ����\���.
	 @return true - ���� �������� ���������.
      */
      bool setDSA( const double& value );
    
      /**
	 ��������� ���� ������� �� ���� �����, �������.
	 ������������ �������� DSE = 10 ����\���.
	 @return true - ���� �������� ���������.
      */
      bool setDSE(const double& value );
    
      double azimut() { return m_azimut; }
      double elevat() { return m_elevat; }

      /** ��������� �������� ��� �������. */
      double nextAzimut();
    
      /** ��������� �������� ��� ���� �����. */
      double nextElevat();
    
    };
  }
}


#endif //_MANTPROGON_
