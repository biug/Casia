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
	-比如对workermanager进行修改，需要创建/切换到worker branch下
--开发功能完毕merge到develop分支，不要merge到master分支
--merge之前需要做代码审核
--每次提交要有良好的comment，不要出现“我也不知道改了什么”“fix bug”这样的comment
--每周末merge到一次master上进行总测试
