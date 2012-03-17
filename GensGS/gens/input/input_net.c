void Scan_Player_Net(int Player)
{
#if 0
  if (!Player)
    return;

  Update_Input ();

  if (Check_Key_Pressed (Keys_Def[0].Up))
    {
      Kaillera_Keys[0] &= ~0x08;
      Kaillera_Keys[0] |= 0x04;
    }
  else
    {
      Kaillera_Keys[0] |= 0x08;
      if (Check_Key_Pressed (Keys_Def[0].Down))
	Kaillera_Keys[0] &= ~0x04;
      else
	Kaillera_Keys[0] |= 0x04;
    }

  if (Check_Key_Pressed (Keys_Def[0].Left))
    {
      Kaillera_Keys[0] &= ~0x02;
      Kaillera_Keys[0] |= 0x01;
    }
  else
    {
      Kaillera_Keys[0] |= 0x02;
      if (Check_Key_Pressed (Keys_Def[0].Right))
	Kaillera_Keys[0] &= ~0x01;
      else
	Kaillera_Keys[0] |= 0x01;
    }

  if (Check_Key_Pressed (Keys_Def[0].Start))
    Kaillera_Keys[0] &= ~0x80;
  else
    Kaillera_Keys[0] |= 0x80;

  if (Check_Key_Pressed (Keys_Def[0].A))
    Kaillera_Keys[0] &= ~0x40;
  else
    Kaillera_Keys[0] |= 0x40;

  if (Check_Key_Pressed (Keys_Def[0].B))
    Kaillera_Keys[0] &= ~0x20;
  else
    Kaillera_Keys[0] |= 0x20;

  if (Check_Key_Pressed (Keys_Def[0].C))
    Kaillera_Keys[0] &= ~0x10;
  else
    Kaillera_Keys[0] |= 0x10;

  if (Controller_1_Type & 1)
    {
      if (Check_Key_Pressed (Keys_Def[0].Mode))
	Kaillera_Keys[1] &= ~0x08;
      else
	Kaillera_Keys[1] |= 0x08;

      if (Check_Key_Pressed (Keys_Def[0].X))
	Kaillera_Keys[1] &= ~0x04;
      else
	Kaillera_Keys[1] |= 0x04;

      if (Check_Key_Pressed (Keys_Def[0].Y))
	Kaillera_Keys[1] &= ~0x02;
      else
	Kaillera_Keys[1] |= 0x02;

      if (Check_Key_Pressed (Keys_Def[0].Z))
	Kaillera_Keys[1] &= ~0x01;
      else
	Kaillera_Keys[1] |= 0x01;
    }
#endif
}


void Update_Controllers_Net(int num_player)
{
#if 0
  Controller_1_Up = (Kaillera_Keys[0] & 0x08) >> 3;
  Controller_1_Down = (Kaillera_Keys[0] & 0x04) >> 2;
  Controller_1_Left = (Kaillera_Keys[0] & 0x02) >> 1;
  Controller_1_Right = (Kaillera_Keys[0] & 0x01);
  Controller_1_Start = (Kaillera_Keys[0] & 0x80) >> 7;
  Controller_1_A = (Kaillera_Keys[0] & 0x40) >> 6;
  Controller_1_B = (Kaillera_Keys[0] & 0x20) >> 5;
  Controller_1_C = (Kaillera_Keys[0] & 0x10) >> 4;

  if (Controller_1_Type & 1)
    {
      Controller_1_Mode = (Kaillera_Keys[0 + 1] & 0x08) >> 3;
      Controller_1_X = (Kaillera_Keys[0 + 1] & 0x04) >> 2;
      Controller_1_Y = (Kaillera_Keys[0 + 1] & 0x02) >> 1;
      Controller_1_Z = (Kaillera_Keys[0 + 1] & 0x01);
    }

  if (num_player > 2)		// TEAMPLAYER
    {
      Controller_1B_Up = (Kaillera_Keys[2] & 0x08) >> 3;
      Controller_1B_Down = (Kaillera_Keys[2] & 0x04) >> 2;
      Controller_1B_Left = (Kaillera_Keys[2] & 0x02) >> 1;
      Controller_1B_Right = (Kaillera_Keys[2] & 0x01);
      Controller_1B_Start = (Kaillera_Keys[2] & 0x80) >> 7;
      Controller_1B_A = (Kaillera_Keys[2] & 0x40) >> 6;
      Controller_1B_B = (Kaillera_Keys[2] & 0x20) >> 5;
      Controller_1B_C = (Kaillera_Keys[2] & 0x10) >> 4;

      if (Controller_1B_Type & 1)
	{
	  Controller_1B_Mode = (Kaillera_Keys[2 + 1] & 0x08) >> 3;
	  Controller_1B_X = (Kaillera_Keys[2 + 1] & 0x04) >> 2;
	  Controller_1B_Y = (Kaillera_Keys[2 + 1] & 0x02) >> 1;
	  Controller_1B_Z = (Kaillera_Keys[2 + 1] & 0x01);
	}

      Controller_1C_Up = (Kaillera_Keys[4] & 0x08) >> 3;
      Controller_1C_Down = (Kaillera_Keys[4] & 0x04) >> 2;
      Controller_1C_Left = (Kaillera_Keys[4] & 0x02) >> 1;
      Controller_1C_Right = (Kaillera_Keys[4] & 0x01);
      Controller_1C_Start = (Kaillera_Keys[4] & 0x80) >> 7;
      Controller_1C_A = (Kaillera_Keys[4] & 0x40) >> 6;
      Controller_1C_B = (Kaillera_Keys[4] & 0x20) >> 5;
      Controller_1C_C = (Kaillera_Keys[4] & 0x10) >> 4;

      if (Controller_1C_Type & 1)
	{
	  Controller_1C_Mode = (Kaillera_Keys[4 + 1] & 0x08) >> 3;
	  Controller_1C_X = (Kaillera_Keys[4 + 1] & 0x04) >> 2;
	  Controller_1C_Y = (Kaillera_Keys[4 + 1] & 0x02) >> 1;
	  Controller_1C_Z = (Kaillera_Keys[4 + 1] & 0x01);
	}

      Controller_1D_Up = (Kaillera_Keys[6] & 0x08) >> 3;
      Controller_1D_Down = (Kaillera_Keys[6] & 0x04) >> 2;
      Controller_1D_Left = (Kaillera_Keys[6] & 0x02) >> 1;
      Controller_1D_Right = (Kaillera_Keys[6] & 0x01);
      Controller_1D_Start = (Kaillera_Keys[6] & 0x80) >> 7;
      Controller_1D_A = (Kaillera_Keys[6] & 0x40) >> 6;
      Controller_1D_B = (Kaillera_Keys[6] & 0x20) >> 5;
      Controller_1D_C = (Kaillera_Keys[6] & 0x10) >> 4;

      if (Controller_1D_Type & 1)
	{
	  Controller_1D_Mode = (Kaillera_Keys[6 + 1] & 0x08) >> 3;
	  Controller_1D_X = (Kaillera_Keys[6 + 1] & 0x04) >> 2;
	  Controller_1D_Y = (Kaillera_Keys[6 + 1] & 0x02) >> 1;
	  Controller_1D_Z = (Kaillera_Keys[6 + 1] & 0x01);
	}
    }
  else
    {
      Controller_2_Up = (Kaillera_Keys[2] & 0x08) >> 3;
      Controller_2_Down = (Kaillera_Keys[2] & 0x04) >> 2;
      Controller_2_Left = (Kaillera_Keys[2] & 0x02) >> 1;
      Controller_2_Right = (Kaillera_Keys[2] & 0x01);
      Controller_2_Start = (Kaillera_Keys[2] & 0x80) >> 7;
      Controller_2_A = (Kaillera_Keys[2] & 0x40) >> 6;
      Controller_2_B = (Kaillera_Keys[2] & 0x20) >> 5;
      Controller_2_C = (Kaillera_Keys[2] & 0x10) >> 4;

      if (Controller_2_Type & 1)
	{
	  Controller_2_Mode = (Kaillera_Keys[2 + 1] & 0x08) >> 3;
	  Controller_2_X = (Kaillera_Keys[2 + 1] & 0x04) >> 2;
	  Controller_2_Y = (Kaillera_Keys[2 + 1] & 0x02) >> 1;
	  Controller_2_Z = (Kaillera_Keys[2 + 1] & 0x01);
	}
    }
#endif
}
