static void process_command(uint8_t key)
{
  uint8_t temp;

  switch(key)
  {
   case CMDMODE_ABORT_KEY:
    /* the official way to exit command mode */
    break;
   case KEY_1:
   case KEY_2:
   case KEY_3:
   case KEY_4:
   case KEY_5:
   case KEY_6:
   case KEY_7:
   case KEY_8:
   case KEY_9:
   case KEY_0:
    /* set key map 0..9 */
    if(key == KEY_0) temp=0;
    else             temp=key-KEY_1+1;
    if(temp != get_current_keymap_index()) set_current_keymap(temp);
    break;
   default:
    break;
  }
}
