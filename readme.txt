To Compile:
- Open src/CasiaBot/VisualStudio/CasiaBot.sln in VS2017
- Select Release mode
- Build the CasiaBot project
- Output will go to src/CasiaBot/bin/CasiaBot.dll

Tournament Setup:
- Copy dll/CasiaBot.dll (or the above compiled dll) to the tournament CasiaBot/AI folder
- Copy dll/CasiaBot_Config.txt to the tournament CasiaBot/AI folder

Casia Flow Note:
--每个人添加功能时需要在相应的feature分支上进行修改
	-功能分支以‘feature-’为开头，对应相应的功能描述
	-比如对workermanager进行修改，需要创建feature-worker分支，并切换到feature-worker分支下进行修改
--开发功能完毕，提交代码审核请求，检查完毕后可以merge到develop分支，不要merge到master分支
--每次提交要有良好的comment，不要出现“我也不知道改了什么”“fix bug”这样的comment
--定期pull更新代码仓库，保证本地代码最新
--每周末merge到一次master上进行总测试
