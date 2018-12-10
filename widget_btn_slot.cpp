#include "Widget.h"


#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <bd_type.h>
#include <network_job.h>
#include <network_data_process.h>
#include <bdthread.h>
#include <nwthread.h>
#include <Http.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "keyBoard.h"
#include <QDateTime>
#include <dlfcn.h>

/****************************************************
 * *
 * *               widget_B2Indentity
 * *
 * *************************************************/

// slot...btn_B2Indentity_clicked
void Widget::on_btn_B2Indentity_clicked()
{

        // worker thread send singal
//       QString ret = edit_B2Indentity->text();
//       if(ret.isEmpty())
//       {
//           generate_WaringMsb("您还没有输入认证码！");
//           return;
//       }
//       if (ret == "123456")
//       {
//           g_label->show();
//           g_label->setText("认证成功！");
//           m_timer_B2->start(1000);

//           sys_param.status.b2indentity = "yes";
//           J_Config.config_save(sys_param);

//       }
//       else
//       {
//           g_label->show();
//           g_label->setText("认证失败！");
//           generate_WaringMsb("认证码错误！请重新输入...");

//           edit_B2Indentity->clear();
//           edit_B2Indentity->setFocus();
//       }

//       show_label("正在认证...");

//       Http* pHttpFun = new Http();
//       QString strUrl = "http://192.168.1.2/cgi/user.cgi?req=1016";
//       connect(pHttpFun,SIGNAL(signal_requestFinished(bool,const QString&)), //http请求结束信号
//               this,SLOT(slot_B2CheckStatus(bool,const QString&)));
//       pHttpFun->get(strUrl);



       //////

      QString ret = edit_B2Indentity->text();
      if(ret.isEmpty())
      {

          generate_WaringMsb("您还没有输入认证码！");
          return;
      }
      qDebug() << "edit_B2Indentity input: " << ret;
     if (ret == "123456")
     {
         // secret door
//         g_label->show();
//         g_label->setText("认证成功！");
//         m_timer_B2->start(1000);
         switch_by_stage();

         //sys_param.status.b2indentity = "yes";
         //J_Config.config_save(sys_param);
     }
     else
     {
         // real verify
         qDebug() << "slot_ipsec_pwdverify";

         void *handle;
         char *error;
         typedef unsigned int (*tf_get_error_t)(int, char *,int);
         typedef int (*ipsec_pwdverify_t)(unsigned char *);
         handle = dlopen ("libcall.so", RTLD_NOW);
         if (!handle)
         {
             fprintf (stderr, "%s err\n", dlerror());
             exit(1);
         }
         dlerror();
         ipsec_pwdverify_t ipsec_pwdverify=(ipsec_pwdverify_t)dlsym(handle,"ipsec_pwdverify" );
         tf_get_error_t tf_get_error=(tf_get_error_t)dlsym(handle,"tf_get_error" );
         if ((error = dlerror()) != NULL)
         {
             fprintf (stderr, "%s\n", error);
             exit(1);
         }
         //char* pwd = ret;
         char * pwd = "12345678";
         int result = ipsec_pwdverify((unsigned char *)ret.toLatin1().data());//通过指针pf的调用来调用动态库中的test函数
         qDebug() << "ipsec_pwdverify result:" << result ;


         if(0 != result)
         {
             // if not ok, show the error reason

             char ErrorString[100];
             unsigned int  error_rtn = tf_get_error(result,ErrorString,50);

             qDebug() << "error_rtn:" << error_rtn << " ErrorString:" << ErrorString;

            g_label->show();
            g_label->setText("认证失败！");
            //generate_WaringMsb("认证码错误！请重新输入...");
            generate_WaringMsb(QString(ErrorString));

            edit_B2Indentity->clear();
            edit_B2Indentity->setFocus();
         }
         else if(sys_param.status.b2indentity != "yes")
         {
            // if ok and first verify, switch to changepwd page
             widget_resume_index = widget_B2Indentity_index;

              s_Layout->setCurrentIndex(widget_B2ChangePwd_index);

             sys_param.status.b2indentity = "yes";
             J_Config.config_save(sys_param);
         }
         else
         {
            // if ok and not first verify, switch to specific page according to config.json
            switch_by_stage();
         }


         dlclose(handle);      //关闭调用动态库句柄
     }







}




/****************************************************
 * *
 * *               widget_InfoReport
 * *
 * *************************************************/
//slot...btn_readCardclicked.
void Widget::on_btn_readCard_clicked()
{
    if(isReadCard == false)
    {
        BDThread::readCARD();
    }
}
//slot...btn_InfoReport_clicked.
void Widget::on_btn_InfoReport_clicked()
{

    QString bdid = edit_bdID->text();
    QString devid = edit_devID->text();

    if(isReadCard == false)
    {
        generate_WaringMsb("请读取北斗卡号!");
        return;
    }

    // to be continued ... http send
    QString strUrl = "http://27.128.164.177:9090/xxxgw/boxid/report?devid="
            + devid + "&bdid=" + bdid;

    Http* pHttpFun = new Http();
    connect(pHttpFun,SIGNAL(signal_requestFinished(bool,const QString&)), //http请求结束信号
            this,SLOT(slot_InfoReportResult(bool,const QString&)));
    pHttpFun->get(strUrl);
    qDebug()<< "I=================================================I";
    qDebug()<< "InfoReport send"<<strUrl;
    g_label->show();
    g_label->clear();
    g_label->setText("信息上报中，请稍后...");
}


/****************************************************
 * *
 * *               widget_TaskGet
 * *
 * *************************************************/

//slot..btn_taskGet_clicked
void Widget::on_btn_taskGet_clicked()
{
    qDebug() << "slot_requestBtnClicked";

    Http* pHttpFun = new Http();
    QString strUrl = "http://27.128.164.177:9090/xxxgw/agw/gettask?devid=" + sys_param.devid;
    connect(pHttpFun,SIGNAL(signal_requestFinished(bool,const QString&)), //http请求结束信号
            this,SLOT(slot_taskGetResult(bool,const QString&)));
    pHttpFun->get(strUrl);

    widget_resume_index = s_Layout->currentIndex();
    label_tipDialog->setText("正在获取任务信息，请稍候...");
    s_Layout->setCurrentIndex(widget_tipDialog_index);
}


//slot..btn_perLogin_clicked
void Widget::on_btn_perLogin_clicked()
{
    int index =( s_Layout->currentIndex() + 1 );
    s_Layout->setCurrentIndex(index);
}



/****************************************************
 * *
 * *               widget_Login
 * *
 * *************************************************/

//slot...btn_login_clicked
void Widget::on_btn_login_clicked()
{
    qDebug() << "on_btn_login_clicked";


    QString userName = edit_userName->text();
    QString userPwd = edit_userPwd->text();
    if(isFirstLogin)
    {

        widget_resume_index = s_Layout->currentIndex();
        label_tipDialog->setText("正在登录，请稍候...");
        s_Layout->setCurrentIndex(widget_tipDialog_index);


        m_ui_param->sess_name = userName;
        m_ui_param->sess_pwd = userPwd;

        int count = m_ui_param->executeor_list.count();
        int current=0;
        for(current=0; current<count; current++)
        {
            if(userName == m_ui_param->executeor_list[current])
            {
                // create nwThread and start
                nwthread = new NWThread(this,userName, userPwd);
                connect(nwthread,SIGNAL(signal_login_result(bool,QString)),this,SLOT(slot_loginResult(bool,QString)));
                connect(nwthread,SIGNAL(signal_nw_data_avaliable(QString)),this,SLOT(slot_4G_Data(QString)));
                nwthread->start();
                break;
            }
        }
        if(current == count)
        {
            generate_WaringMsb("用户与任务不匹配！");
            s_Layout->setCurrentIndex(widget_Login_index);
        }
/*
        if(userName == m_ui_param->executor_code)
        {
            // create nwThread and start
            nwthread = new NWThread(this,userName, userPwd);
            connect(nwthread,SIGNAL(signal_login_result(bool,QString)),this,SLOT(slot_loginResult(bool,QString)));
            connect(nwthread,SIGNAL(signal_nw_data_avaliable(QString)),this,SLOT(slot_4G_Data(QString)));
            nwthread->start();
        }
        else
        {
            generate_WaringMsb("用户与任务不匹配！");
            s_Layout->setCurrentIndex(widget_resume_index);
        }
*/
    }
    else
    {
        int count = m_ui_param->executeor_list.count();
        int current=0;
        for(current=0; current<count; current++)
        {
            if(userName == m_ui_param->executeor_list[current])
            {
                generate_InfoMsb("登录成功！");
                if(isFirstFingerPrint){
                    s_Layout->setCurrentIndex(widget_FingerPrint_index);
                }
                else{
                    s_Layout->setCurrentIndex(widget_main_index);
                }
            }
        }
        if(current == count)
        {
            generate_WaringMsb("用户与任务不匹配！");
            s_Layout->setCurrentIndex(widget_Login_index);
        }
    }

        /*
        widget_resume_index = s_Layout->currentIndex();
        label_tipDialog->setText("正在登录，请稍候...");
        s_Layout->setCurrentIndex(widget_tipDialog_index);

        if((edit_userName->text() == PrccessParam.name) && (edit_userPwd->text() == PrccessParam.pwd))
        {
             generate_InfoMsb("登录成功！");
             if(isFirstFingerPrint)
             {
                s_Layout->setCurrentIndex(widget_FingerPrint_index);
             }
             else
             {
                 s_Layout->setCurrentIndex(widget_main_index);
             }
        }
        else
        {
            generate_WaringMsb("登录失败！请重试...");
            s_Layout->setCurrentIndex(widget_Login_index);
        }

        */

}

//slot...btn_loginCancel_clicked
void Widget::on_btn_loginCancel_clicked()
{
    edit_userName->clear();
    edit_userName->setFocus();
    edit_userPwd->clear();
}



/****************************************************
 * *
 * *               widget_FingerPrint
 * *
 * *************************************************/

void Widget::on_btn_fingerPrint1_clicked()
{
//    generate_InfoMsb("第一次录入成功！");
//    set_btn_icon(btn_fingerPrint1,"icon_fingerprint_yes");
//    fp1_success = true;
}

void Widget::on_btn_fingerPrint2_clicked()
{
//     generate_InfoMsb("第二次录入成功！");
//     set_btn_icon(btn_fingerPrint2,"icon_fingerprint_yes");
//     fp2_success = true;
}

void Widget::on_btn_fingerPrint3_clicked()
{
//    generate_InfoMsb("第三次录入成功！");
//    set_btn_icon(btn_fingerPrint3,"icon_fingerprint_yes");
//    fp3_success = true;
//    if(fp1_success==true && fp2_success==true && fp3_success == true)
//    {
//        fp1_success = false;
//        fp2_success = false;
//        fp3_success = false;

//        g_label->show();
//        g_label->setText("指纹录入成功！");
//        m_timer_fingerPrint->start(1000);
    //    }
}

void Widget::on_btn_fingerPrint_continue_clicked()
{
     suoThread::suo_control("$FINGE");
}

void Widget::on_btn_fingerPrint_end_clicked()
{
    if(fingerPrint_success == true)
    {
        s_Layout->setCurrentIndex(widget_main_index);
        hide_label();
    }
    else
    {
        generate_WaringMsb("请先录入指纹！");
    }
}


/****************************************************
 * *
 * *               widget_main
 * *
 * *************************************************/
//slot..on_btn_go_to_work_clicked
void Widget::on_btn_go_to_work_clicked()
{
    autoLockScreen_stop();
    s_Layout->setCurrentIndex(widget_work_index);
}

//slot..on_btn_lock_screen_clicked
void Widget::on_btn_lock_screen_clicked()
{
    lock_screen();
    autoLockScreen_stop();
}




/****************************************************
 * *
 * *               widget_work
 * *
 * *************************************************/
void Widget::on_btn_addition_clicked()
{
    s_Layout->setCurrentIndex(widget_addition_index);
}

//slot..4G_unlock_clicked
void Widget::on_btn_unlock4G_clicked()
{
    if(m_ui_param->feixingmode=="flyon")
    {
        generate_WaringMsb("当前处于飞行模式！");
        return;
    }

//    autoLockScreen_stop();
//    if(task_status == TASK_RUNNING)
//    {
//        int ret= generate_SelectMsb("你确定要使用4G解锁吗？");
//        if(ret == QMessageBox::Ok)
//        {
//            widget_resume_index = s_Layout->currentIndex();
//            label_tipDialog->setText("正在获取开锁码，请稍候...");
//            s_Layout->setCurrentIndex(widget_tipDialog_index);


//            QString msg = Network_Job::reqcode_cmd_send();
//            nwthread->nw_send_data(msg);
//            m_timer_unlock4G->start(30000); // timer 30s
//            m_timer_1s->start(1000);
//        }
//    }
//    if(task_status == TASK_PAUSE)
//    {
//        generate_WaringMsb("任务已暂停！请先恢复任务...");
//    }
    qDebug() <<"on_btn_unlock4G_clicked------";
    autoLockScreen_stop();
    if(Check_Taskmode()=="special")
    {
        s_Layout->setCurrentIndex(widget_Supercode_unlock_index);

    }else
    {

    if(task_status == TASK_RUNNING)
    {
        int ret= generate_SelectMsb("你确定要使用4G解锁吗？");
        if(ret == QMessageBox::Ok)
        {
            widget_resume_index = s_Layout->currentIndex();
            label_tipDialog->setText("正在获取开锁码，请稍候...");
            s_Layout->setCurrentIndex(widget_tipDialog_index);


            QString msg = Network_Job::reqcode_cmd_send();
            nwthread->nw_send_data(msg);
            m_timer_unlock4G->start(30000); // timer 30s
            m_timer_1s->start(1000);
        }
    }
    if(task_status == TASK_PAUSE)
    {
        generate_WaringMsb("任务已暂停！请先恢复任务...");
    }
    }
}

//slot..BD_unlock_clicked
void Widget::on_btn_unlockBD_clicked()
{
    if(m_ui_param->feixingmode=="flyon")
    {
        generate_WaringMsb("当前处于飞行模式！");
        return;
    }
//    autoLockScreen_stop();
//    if(task_status == TASK_RUNNING)
//    {
//        int ret= generate_SelectMsb("你确定要使用北斗解锁吗？");
//        if(ret == QMessageBox::Ok)
//        {
//            BDThread::bd_unlock_cmd();

//            widget_resume_index = s_Layout->currentIndex();
//            label_tipDialog->setText("正在获取开锁码，请稍候...");
//            s_Layout->setCurrentIndex(widget_tipDialog_index);
//            m_timer_unlockBD->start(30000);     // timer 30s
//            m_timer_1s->start(1000);
//        }
//    }
//    if(task_status == TASK_PAUSE)
//    {
//        generate_WaringMsb("任务已暂停！请先恢复任务...");
//    }
    qDebug() <<"on_btn_unlockBD_clicked------";
autoLockScreen_stop();
if(Check_Taskmode()=="special")
{
    s_Layout->setCurrentIndex(widget_FingerUnlock_index);
}else
{
    if(task_status == TASK_RUNNING)
    {
        int ret= generate_SelectMsb("你确定要使用北斗解锁吗？");
        if(ret == QMessageBox::Ok)
        {
            BDThread::bd_unlock_cmd();

            widget_resume_index = s_Layout->currentIndex();
            label_tipDialog->setText("正在获取开锁码，请稍候...");
            s_Layout->setCurrentIndex(widget_tipDialog_index);
            m_timer_unlockBD->start(30000);     // timer 30s
            m_timer_1s->start(1000);
        }
    }
    if(task_status == TASK_PAUSE)
    {
        generate_WaringMsb("任务已暂停！请先恢复任务...");
    }
}
}

//slot..btn_taskPause_clicked
void Widget::on_btn_taskPause_clicked()
{
    autoLockScreen_stop();

    s_Layout->setCurrentIndex(widget_taskPause_index);

//    int ret= generate_SelectMsb("你确定要暂停任务吗？");
//    if(ret == QMessageBox::Ok)
//    {
//        widget_resume_index = s_Layout->currentIndex();
//        label_tipDialog->setText("正在暂停任务，请稍候...");
//        s_Layout->setCurrentIndex(widget_tipDialog_index);

//        QString msg = Network_Job::taskpau_cmd_send();
//        nwthread->nw_send_data(msg);
//        m_timer_taskPause->start(30000); // timer 30s
//        m_timer_1s->start(1000);
//    }
}

//slot..btn_taskResume_clicked
void Widget::on_btn_taskResume_clicked()
{
    autoLockScreen_stop();
    int ret= generate_SelectMsb("你确定要恢复任务吗？");
    if(ret == QMessageBox::Ok)
    {


        label_tipDialog->setText("正在恢复任务，请稍候...");
        s_Layout->setCurrentIndex(widget_tipDialog_index);

        m_timer_taskResume->start(40*1000);   //timer 40s
        m_timer_1s->start(1000);

        QString userName = PrccessParam.sess_name;
        QString userPwd = PrccessParam.sess_pwd;
        nwthread = new NWThread(this,userName, userPwd);
        connect(nwthread,SIGNAL(signal_login_result(bool,QString)),this,SLOT(slot_reloginResult(bool,QString)));
        connect(nwthread,SIGNAL(signal_nw_data_avaliable(QString)),this,SLOT(slot_4G_Data(QString)));
        nwthread->start();

    }
}


//slot..btn_taskComplete_clicked
void Widget::on_btn_taskComplete_clicked()
{
    autoLockScreen_stop();
    if(task_status == TASK_RUNNING)
    {
        int ret= generate_SelectMsb("你确定要完成任务吗？");
        if(ret == QMessageBox::Ok)
        {
            label_tipDialog->setText("正在发起任务完成请求，请稍候...");
            s_Layout->setCurrentIndex(widget_tipDialog_index);

            QString msg = Network_Job::taskdone_cmd_send();
            nwthread->nw_send_data(msg);

            m_timer_taskComplete->start(30000);
            m_timer_1s->start(1000);

        }
    }
    if(task_status == TASK_PAUSE)
    {
        generate_WaringMsb("任务已暂停！请先恢复任务...");
    }
}

//slot..btn_go_to_main_clicked
void Widget::on_btn_go_to_main_clicked()
{
    autoLockScreen_stop();
    s_Layout->setCurrentIndex(widget_main_index);
}






// slot..on_btn_infoBT_clicked
void Widget::on_btn_infoBT_clicked()
{

}

//slot..btn_lock_screen2_clicked
void Widget::on_btn_lock_screen2_clicked()
{
    lock_screen();
    autoLockScreen_stop();
}

//slot..btn_set_clicked
void Widget::on_btn_set_clicked()
{
    autoLockScreen_stop();
    generate_InfoMsb("功能升级中，敬请期待！");

    //    QApplication::quit();
}

void Widget::on_btn_closeLock_clicked()
{
    autoLockScreen_stop();

    int ret= generate_SelectMsb("你确定要手动关锁吗？");
    if(ret == QMessageBox::Ok)
    {
        isSendUnlockLog = false;
        suoParam.Suo_State = SUO_CLOSE;
        suoThread::suo_control("$OLOCK");
    }
}

/****************************************************
 * *
 * *               widget_BsiInfo
 * *
 * *************************************************/

// slot..on_btn_infoBsiClose_clicked
void Widget::on_btn_infoBsiClose_clicked()
{
    flag_BsiInfo = false;
    s_Layout->setCurrentIndex(widget_work_index);
}

/****************************************************
 * *
 * *               widget_unlockScreen
 * *
 * *************************************************/
// slot..btn_unlockScreen_clicked
void Widget::on_btn_unlockScreen_clicked()
{
    autoLockScreen_stop();
    QString userName = edit_lsUserName->text();
    QString userPwd = edit_lsUserPwd->text();

    // 首先判断是否为任务执行人
    int count = m_ui_param->executeor_list.count();
    int current;
    for(current=0; current<count; current++)
    {
        if(userName == m_ui_param->executeor_list[current])
        {
            //若是上次登录人员，无需发送awake
            if(userName == PrccessParam.sess_name )
            {
                if(userPwd == PrccessParam.sess_pwd)
                {
                    s_Layout->setCurrentIndex(widget_main_index);
                    isLockScreen = false;

                    edit_lsUserPwd->clear();
                    edit_lsUserName->clear();
                }
                else
                {
                    generate_WaringMsb("密码错误，请重新输入...");
                    isLockScreen = false;
                    edit_lsUserPwd->clear();
                    edit_lsUserName->clear();
                }
            }
            // 若不是上次登录人员，发送awake协议
            else
            {
                label_tipDialog->setText("正在登录，请稍候...");
                s_Layout->setCurrentIndex(widget_tipDialog_index);

                QString msg = Network_Job::awake_cmd_send(userName,userPwd);
                nwthread->nw_send_data(msg);
                //临时保存sess_name sess_pwd
                m_ui_param->sess_name = userName;
                m_ui_param->sess_pwd = userPwd;

                isLockScreen = false;
                edit_lsUserPwd->clear();
                edit_lsUserName->clear();

                m_timer_awake->start(30000); // timer 30s
                m_timer_1s->start(1000);
            }

            break;
        }
    }

    // 无效的任务执行人
    if(current == count)
    {
        generate_WaringMsb("用户与任务不匹配！");
    }
}

// slot..btn_unlockScreen_cancle_clicked
void Widget::on_btn_unlockScreen_cancle_clicked()
{
    edit_lsUserPwd->clear();
    edit_lsUserName->clear();
    lock_screen();
    autoLockScreen_stop();
}

/****************************************************
 * *
 * *               widget_blackScreen
 * *
 * *************************************************/
// slot..btn_blackScreen_clicked
void Widget::on_btn_blackScreen_clicked()
{
    m_double_click++;
    if( m_double_click ==1 )
    {
        m_timer_doubleClick->start(200);
    }
    if( m_double_click == 2 )
    {
        m_double_click = 0;
        m_timer_doubleClick->stop();

        // 处理。。。。
        // set background to normal
//        set_background(this,"./Resource/background1.png");
//        s_Layout->setCurrentIndex(widget_unlockScreen_index);
    }
}

/****************************************************
 * *
 * *               widget_taskPause
 * *
 * *************************************************/
void Widget::on_btn_taskPause_confirm_clicked()
{
    QString taskPause_reson = comboBox_taskPause_reson->currentText();

    widget_resume_index = s_Layout->currentIndex();
    label_tipDialog->setText("正在暂停任务，请稍候...");
    s_Layout->setCurrentIndex(widget_tipDialog_index);

    QString msg = Network_Job::taskpau_cmd_send(taskPause_reson);
    nwthread->nw_send_data(msg);
    m_timer_taskPause->start(30000); // timer 30s
    m_timer_1s->start(1000);
}

void Widget::on_btn_taskPause_cancle_clicked()
{
    s_Layout->setCurrentIndex(widget_work_index);
}


/****************************************************
 * *
 * *               widget_phoneNumber
 * *
 * *************************************************/


void Widget::on_btn_phoneNumber1_clicked()
{
    lock_screen();
    autoLockScreen_stop();
}

void Widget::on_btn_phoneNumber2_clicked()
{

}



/****************************************************
 * *
 * *               widget_inputDyncode
 * *
 * *************************************************/

void Widget::on_btn_dyncode_unlock_clicked()
{
    QString str = edit_dyncode->text().trimmed();

    if(str == m_ui_param->dyncode)
    {
        isSendUnlockLog = true;
        suoParam.Suo_State = SUO_OPEN;

        PrccessParam.unlocktype = "unlock";

        suoThread::suo_control("$OLOCK");

        show_label("正在开锁，请稍候...");
        edit_dyncode->clear();
    }
    else
    {
        generate_WaringMsb("解锁码错误! 请重试...");
        edit_dyncode->clear();
    }
}

void Widget::on_btn_dyncode_cancel_clicked()
{
    s_Layout->setCurrentIndex(widget_work_index);
}

/****************************************************
 * *
 * *               widget_warning_show
 * *
 * *************************************************/

void Widget::on_btn_warning_toggle_clicked()
{
    autoLockScreen_stop();
    btn_warning_toggle->hide();
    m_timer_warning_blink->stop();

    QString ret;
    for(int i=0; i<warning_stringList.count(); i++)
    {
//        qDebug()<<warning_stringList[i];
        ret.append(warning_stringList[i]);
    }
    test_edit->setPlainText(ret);
    test_edit->moveCursor(QTextCursor::End);

    if( s_Layout->currentIndex() != widget_warning_show_index)
    {
        widget_resume_index = s_Layout->currentIndex();
    }
    else
    {
//        widget_resume_index = widget_main_index;
    }
    s_Layout->setCurrentIndex(widget_warning_show_index);
}

void Widget::on_btn_warning_confirm_clicked()
{
    s_Layout->setCurrentIndex(widget_resume_index);

//    widget_resume_index = s_Layout->currentIndex();
//    s_Layout->setCurrentIndex(widget_warning_show_index);

//    QString ret;
//    for(int i=0; i<warning_stringList.count(); i++)
//    {
////        qDebug()<<warning_stringList[i];
//        ret.append(warning_stringList[i]);
//    }
//    test_edit->setPlainText(ret);
    //    test_edit->moveCursor(QTextCursor::End);
}




/****************************************************
 * *
 * *               widget_addition
 * *
 * *************************************************/
void Widget::on_btn_addFingerPrint_clicked()
{
     s_Layout->setCurrentIndex(widget_FingerPrint_index);
     suoThread::suo_control("$FINGE");
}

// slot..on_btn_infoBsi_clicked
void Widget::on_btn_infoBsi_clicked()
{
    autoLockScreen_stop();

//    QString msg = Network_Job::warn_cmd_send("lowpower");
//    nwthread->nw_send_data(msg);

    flag_BsiInfo = true;
    s_Layout->setCurrentIndex(widget_BsiInfo_index);
}


void Widget::on_btn_backToWork_clicked()
{
    s_Layout->setCurrentIndex(widget_work_index);
}

void Widget::on_btn_B2ChangePwd_confirm_clicked()
{
    // change pwd
    qDebug() << "slot_ipsec_pwdchange";

    void *handle;
    char *error;
    typedef unsigned int (*tf_get_error_t)(int, char *,int);
    typedef int (*ipsec_pwdchange_t)(unsigned char *,unsigned char *);
    handle = dlopen ("libcall.so", RTLD_NOW);
    if (!handle)
    {
        fprintf (stderr, "%s err\n", dlerror());
        exit(1);
    }
    dlerror();
    ipsec_pwdchange_t ipsec_pwdchange=(ipsec_pwdchange_t)dlsym(handle,"ipsec_pwdchange" );
    tf_get_error_t tf_get_error=(tf_get_error_t)dlsym(handle,"tf_get_error" );
    if ((error = dlerror()) != NULL)
    {
        fprintf (stderr, "%s\n", error);
        exit(1);
    }
    QString pwdold = edit_pwd_old->text();
    QString pwdnew = edit_pwd_new->text();
//    char * pwdold = "old";
//    char * pwdnew = "new";

    if(pwdnew.isEmpty() || pwdold.isEmpty())
    {
        generate_WaringMsb("输入不能为空！");
        dlclose(handle);      //关闭调用动态库句柄
        return;
    }
    else if(pwdnew != pwdold)
    {
        generate_WaringMsb("两次输入不同！");
        dlclose(handle);      //关闭调用动态库句柄
        return;
    }

    qDebug() << "pwdold input: "<< pwdold;
    qDebug() << "pwdnew input: "<< pwdnew;

    int result = ipsec_pwdchange((unsigned char *)pwdold.toLatin1().data(),(unsigned char *)pwdnew.toLatin1().data());
    qDebug() << "ipsec_pwdchange result:" << result ;

    if(0 != result)
    {
        // change fail,show the reason
        char ErrorString[100];
        unsigned int  error_rtn = tf_get_error(result,ErrorString,50);

        qDebug() << "error_rtn:" << error_rtn << " ErrorString:" << ErrorString;

        generate_WaringMsb(QString(ErrorString));

        edit_pwd_old->clear();
        edit_pwd_old->setFocus();
        edit_pwd_new->clear();
    }
    else
    {
        s_Layout->setCurrentIndex(widget_resume_index);
        g_label->setText("口令更改成功！");
        m_timer_B2ChangePwd->start(1000);
    }

    dlclose(handle);      //关闭调用动态库句柄
}

void Widget::on_btn_B2ChangePwd_cancel_clicked()
{
    s_Layout->setCurrentIndex(widget_resume_index);
}


/****************************************************
 * *
 * *               widget_test
 * *
 * *************************************************/
// test widget...
void Widget::on_btn_testReadCard_clicked()
{
    qDebug() << "SUO_OPEN";
    suoParam.Suo_State = SUO_OPEN;
    suoThread::suo_control("$OLOCK");
}

void Widget::on_btn_testReadBsi_clicked()
{
    suoParam.Suo_State = SUO_CLOSE;
    suoThread::suo_control("$OLOCK");
}

void Widget::on_btn_testInfoRS_clicked()
{
    suoThread::suo_control("$FINGE");
}

void Widget::on_btn_testReadLocation_clicked()
{
    suoThread::suo_control("$CLEAR");
}


// slot...btn4 clicked
void Widget::on_m_btn4_clicked()
{
    int index =( s_Layout->currentIndex() - 1 );
    s_Layout->setCurrentIndex(index);
}
//slot...btn5 clicked
void Widget::on_m_btn5_clicked()
{
    int index = s_Layout->currentIndex() + 1;
    s_Layout->setCurrentIndex(index);
}

void Widget::on_btn_Supercode_unlock_clicked()//wy
{
    qDebug()<<"Super open suo";

    isSendUnlockLog = true;
    suoParam.Suo_State = SUO_OPEN;

    PrccessParam.unlocktype = "superunlock";

    suoThread::suo_control("$OLOCK");

    show_label("正在开锁，请稍候...");
    edit_Supercode->clear();


    // send superunlock log to frontsvr



}
void Widget::on_btn_Supercode_cancel_clicked()
{
    s_Layout->setCurrentIndex(widget_work_index);
}

void Widget:: on_btn_FingerUnlock_clicked()//wy
{
    if(m_ui_param->feixingmode=="flyon")
    {
        generate_WaringMsb("当前处于飞行模式！");
        return;
    }
      qDebug()<<"on_btn_FingerUnlock_clicked  open suo";
}
void Widget:: on_btn_FingerCancel_clicked()
{
    s_Layout->setCurrentIndex(widget_work_index);
}
void Widget::on_btn_feixing_clicked()//wy
{
        autoLockScreen_stop();
//        if(m_ui_param->feixingmode=="flyoff")
//        {
//            set_btn_icon(btn_feixing,"icon_feixingoff.png");//wy
//        }else
//        {

//        }
//        generate_InfoMsb("你确定要打开飞行模式吗？");
        int ret= generate_SelectMsb("你确定要打开飞行模式吗？");
//        int ret= generate_SelectMsb("你确定要关闭飞行模式吗？");
        if(ret == QMessageBox::Ok)
        {
            qDebug()<<"open fly mode";
                if(m_ui_param->feixingmode=="flyon")
                {
                    set_btn_icon(btn_feixing,"icon_feixingoff.png");//wy
                    m_ui_param->feixingmode="flyoff";
                }else
                {
                    set_btn_icon(btn_feixing,"icon_feixingon.png");//wy
                    m_ui_param->feixingmode="flyon";
                }
//            isSendUnlockLog = false;
//            suoParam.Suo_State = SUO_CLOSE;
//            suoThread::suo_control("$OLOCK");
        }
}
void Widget::on_btn_b2_cpwd_clicked()//wy
{
    widget_resume_index = widget_addition_index;

     s_Layout->setCurrentIndex(widget_B2ChangePwd_index);

}
void Widget::on_btn_b2_huiyao_clicked()//wy
{
    autoLockScreen_stop();
    int ret= generate_SelectMsb("你确定要毁钥吗？");
    if(ret == QMessageBox::Ok)
    {
        qDebug()<<"B2--- huiyao";
//        g_label->show();
//        g_label->clear();
//        g_label->setText("模拟毁钥");
//        m_timer_infoRpt->start(1000);

    }

}


