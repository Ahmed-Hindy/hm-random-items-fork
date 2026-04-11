// RandomItems.cpp
// Plugin for Hitman Random Items mod. Spawns or adds random items at intervals.

#include "RandomItems.h"

#include <Logging.h>
#include <IconsMaterialDesign.h>
#include <Globals.h>

#include <Glacier/ZGameLoopManager.h>
#include <Glacier/ZItem.h>
#include <Glacier/ZScene.h>
#include <Glacier/THashMap.h>
#include <Glacier/ZInventory.h>
#include <Glacier/ZModule.h>
#include <Glacier/ZContentKitManager.h>

#include <Glacier/ZGameStats.h>
#include <Glacier/SExternalReferences.h>

#include <Hooks.h>

#include <algorithm>

namespace
{
    // Hardcoded repository IDs that should never be added to the random pool.
    const std::vector<std::string> s_BlacklistedRepositoryIds{
        // Ghost items
        "6B55C267-C83D-494E-85DF-5B4C81439BF6", // (Grenade) Flash Ghost
        "BA5CD3A3-9B90-4088-9B5B-25CA5F3B21B5", // (Grenade) Frag Ghost
        "244E6C4B-774C-4F0C-8D4D-27E74D782A7A", // (Grenade) Freeze Ghost
        "ACAAF159-73B4-4E6E-9B75-C8A58F6DFCB0", // Device Ghost Proximity AudioDistraction
        "5569D6C4-2C83-4DF1-B8B1-A2E909D8B310", // Device Ghost Proximity AudioDistraction Armed
        "F6878AF9-2F91-4807-AC04-EDCC0A47491D", // Device Ghost Proximity Explosive
        "C1434198-FC4E-4224-93EF-1EC0ECCB98B9", // Device Ghost Proximity Flash
        "164D2C5A-04E9-4CE0-950A-A0F6ABBFEB09", // Device Ghost Proximity Flash Armed
        "D2258162-3220-433F-9F81-A368BC8B5D13", // Device Ghost Remote AudioDistraction
        "52BBD0BD-61DA-4AB7-BCD5-D1AD0FC977C6", // Device Ghost Remote AudioDistraction Armed
        "67DB1066-066F-4037-97D3-FE7BDB3A403E", // Device Ghost Remote Explosive
        "4DCF6C65-B55E-4975-90A2-A420A7825F6C", // Device Ghost Remote Explosive Armed
        "0EE8312D-E3B8-4488-AA81-F25CD0CE36AD", // Device Ghost Remote Explosive Ver2
        "B396B54D-2939-4E82-A0E6-8CCA2D64CFED", // Device Ghost Remote Flash
        "23250842-1F5D-4DFB-B705-280A732D7371", // Device Ghost Remote Flash Armed
        "45076001-7382-4F8D-BEF2-BDA6BA6CD4BA", // Device Ghost RubberDuck Explosive
        "8AC6B59C-74CE-48FC-8390-4F037F225F2C", // Device Ghost RubberDuck Explosive Armed
        "F301F605-007C-4FE1-AA99-A8CD2CAE033F", // Sieger 300 Ghost

        // Internal, archetype, test, and quest items
        "CE475C6A-439D-49B7-AD54-325A2FAAE5A5", // (TEST) (AR) TAC-4 AR
        "EBC561B5-B368-4583-B5FE-BC961B4AD4E3", // (TEST) (SMG) TAC SMG NEEDLE
        "8E4A9504-3445-44F3-AF03-A8F6F5C72603", // (TEST) (SMG) TAC SMG SMALL
        "08366AEC-7F5B-4C1E-9F47-00EC2DF7D185", // (TEST) (SMG) TAC SMG SMALL COVERT
        "86522179-2CC9-4BAE-AF70-8D42E423918B", // (TEST) BB 20 Covert
        "824497CF-0C96-4826-A00E-5FB3CCF1140B", // ANDROID ONLY SMG Fast
        "1B31E7E1-F2B7-4B61-8859-77B0C27BB94A", // Archetype Coin
        "47A9C179-C9EC-4F37-B03E-706423F2B462", // Archetype Mine
        "E4BF4570-77C7-4F29-A641-00A481EC0FB9", // Archetype Vial
        "2F8DD433-59ED-4836-992E-E180F26D4D65", // Archmesh 1h Knife
        "C41A7041-60BC-4F1F-BBA4-5ED7232E5A6D", // Archmesh 1h Rock
        "4E7D539B-E20E-485B-8ED0-9CC4236800EE", // Archmesh 1h Screwdriver
        "75B8B85C-13DC-4CD6-AE45-88FB921719E2", // ArchMesh 1h Stick
        "78A89786-D42E-4FE2-8267-1BDC7DD47A45", // ArchMesh 1h Sword
        "A4BA06D1-FBF2-4228-93C7-67CB4F87E6AE", // Archmesh 2h Axe
        "47B206F8-FB22-49AE-8655-4B7A6316EE5C", // ArchMesh 2h Stick
        "C0CCE6AF-5328-4D58-9B1D-CD1324712EF6", // ArchMesh 2h Sword
        "AEB60752-6AC7-4A21-AB24-6714A4C29574", // Bottle Archetype
        "682B9017-51EC-4EB1-A80F-5ABB629911B6", // Cup Archetype
        "06A625B1-D76B-4FCF-BB9B-81DE83C6AED2", // FireExtinguisher Archetype
        "5B427C22-3908-4695-80DA-451FB0DCFE22", // Prop_Explosive_Dynamite_Bellini
        "E5B8B236-2E14-413F-B892-D311BAA06733", // Prop_Gadget_Fireworks_Flash
        "AF82349C-259F-4BDD-8BE7-D5FF61695C29", // Prop_Grenade_EmeticGas
        "DF8324F0-1FCD-4744-BEA4-05A19737A7F3", // prop_melee_colombia_magic_staff
        "ACC9D7B8-80F1-4BB0-BA81-3A69B09E0543", // Prop_Melee_Meat_Item
        "83D09198-60CD-407A-8C27-2F37460193E9", // Prop_Melee_Syringe_Serum_WOLVERINE
        "91925780-9595-449B-B685-EB917C36ECF9", // Questitem Fiberwire Jewel Necklace Fake
        "92D68841-8552-40B1-B8A5-C36C6EFDB6B1", // Questitem Fiberwire Jewel Necklace Real
        "6D6E324B-C66B-4FA5-AFEF-B07A0BA7CEC3", // Selfie Config (crashes the game)
        "82642E14-C6D7-43B3-8B9C-396823A2859A", // Sniper Medium Archetype (Classic)
        "7F90421C-FD20-499E-8821-0950CC8995BC", // Tool_McGuffin
        "16D11A6D-8431-4313-8E19-3A5902D87AB0", // USB Archetype

        // NPC and special-mode items
        "16EDB112-58CC-4069-A7DD-EBD258B14044", // (AR) (NPC) Fusil G1-4
        "D8AA6EBA-0CB7-4ED4-AB99-975F2793D731", // (AR) (NPC) Fusil G2
        "BD461AD3-8498-4D59-87D7-208744AC5066", // (AR) Fusil X2000 NPC
        "7D91BCA8-6FDC-4BF1-B621-7A18065CEBDE", // (Dartgun) Cure Filur
        "E7450009-FC7F-423A-A9AB-B37D200246F5", // (Device-Outbreak) Remote Emetic Gas
        "F0B4C86A-7317-4580-9563-FE470BC0A514", // (Filur) Anti-Viral Serum Vial
        "509A2A77-EBDB-44BD-8177-AFBF4EFD88F7", // (Filur) Antidote Herb A (Green)
        "53E3125E-2DC5-488A-B769-FCEB635CE317", // (Filur) Antidote Herb B (Red)
        "84F094AD-5D5C-4BA9-8EC4-C6712513BD05", // (Filur) Antidote Herb C (Blue)
        "25DD97F5-0A8C-4D60-A69E-F36EF343D462", // (Filur) Device Remote Cure Gas
        "093AA374-56F8-4E31-AC4A-37B1D3450C3D", // (Filur) Grenade Cure Gas
        "3C91D04F-2F97-43D8-AC3C-45A2E238EDE1", // (Filur) Injectable Cure
        "13FBC185-FEED-4D02-B700-913ABFF0B379", // (Melee) Sickle Filur
        "3D10FD97-B1F8-4527-85ED-CD3653BD029B", // (Outbreak) AnitVirus Large
        "8473E9C6-4037-4A8C-87F0-23B703663069", // (Outbreak) AnitVirus Medium
        "F75E4353-59C4-4E29-9B1C-A62C3056A870", // (Outbreak) AnitVirus Small
        "2ECC0DDE-5D5F-4961-AF76-E7C63D7D3405", // (Outbreak) Cure Grenade
        "D7890635-7E81-4A72-822E-65990B666D81", // (Outbreak) Cure Grenade Flash
        "34B41544-64FD-4FB5-8EDA-76B1DD12BF14", // (Outbreak) Device Proximity Cure Gas Audio Distraction
        "095AF579-5504-48C5-92D2-CC253C027A06", // (Outbreak) Device Proximity Cure Gas Large
        "C6E845B2-640D-4F83-B912-BDFC34B38D81", // (Outbreak) Device Proximity Cure Gas Smalll
        "7E8BA667-AC70-4591-9BDE-C7D846EACBB2", // (Outbreak) Device Proximity Gas Lethal
        "83A78B07-1C9A-43A5-802D-35D97E537769", // (Outbreak) Device Proximity Gas Sedative
        "419BC68A-A76E-461D-BA92-327B9A25A182", // (Outbreak) Device Proximity Gas Sick
        "2953E9AC-E25B-41AE-AFBF-4A47F86C4F54", // (Pistol) (NPC) Bartoli 75S
        "79E6FBE5-1C1A-4620-AD3F-30533C07AD5F", // (Pistol) Filur
        "55ED7196-2303-4AF6-9FA3-45B691134561", // (Pistol)(NPC) Bartoli 75R
        "F417BFEC-A999-4B2F-ADEF-510323C75CCF", // (Pistol)(NPC) Bartoli 75R Luxurious
        "B5481DE5-6446-46B3-903F-E0040F46B7F0", // (Shotgun) (NPC) Sawed Off Bartoli 12G
        "901A3B51-51A0-4236-BDF2-23D20696B358", // (Shotgun) (NPC) Tactical Bartoli 12G
        "E206ED81-0559-4289-9FEC-E6A3E9D4EE7C", // (SMG) (NPC) HX-10
        "A5D19E9F-8CA3-4C51-9D79-15D3EA2E7771", // (SMG) (NPC) HX-10 ConcreteArt
        "53A4F9BD-5AA8-4B0E-9200-4593C6966F24", // (SMG) (NPC) HX-10 Lambic
        "3FD9825D-8AA5-48E0-97A9-EC8F541F871A", // (SMG) (NPC) HX-7
        "E0DE34CE-F8D1-428B-8B37-0DAE7398BDE3", // (SMG) (NPC) HX-7 Covert
        "D75BEF38-8A65-45F6-9CD1-CA5E23E2F79A", // (SMG) DAK X2 NPC
        "304A4180-46CE-43AC-AF61-F54BBF8A75EB", // (SMG) DAK X2 NPC COVERT
        "BBABD2BD-6E21-4B9B-A361-71BC255FC9B9", // (SMG) DAK X3 NPC COVERT
        "6738E8AD-B8D0-496A-9749-D27A93B40113", // (SMG) NPC_HX_10_MILITIA
        "719BA201-3688-4984-AFB0-81DC2CC95EC1", // (Sniper) NPC Woodsman
        "09D66DE2-C14C-4C5A-9B85-D1C7E9C2A3E1", // GL2 tool pristine crowbar
        "2B2BDDE8-19D2-40DE-A2FA-F2DDF225D040", // GL2 tool pristine lockpick
        "58E6A677-34AB-4962-B6B7-1892C290019A", // GL2 tool pristine wrench
        "5774F9FF-0ED3-44E0-8208-EC29EAC070C1", // GL2 tool rusty crowbar
        "1E1FEA7E-685E-47EE-8DDE-E975CEBD843F", // GL2 tool rusty hairpin
        "4D2ECDE8-79DB-44B1-8F60-7A1A648F7D09", // GL2 tool rusty wrench
        "E5B2A2FB-8655-4462-9F05-3B02D0A86299", // Grays (SMG) (NPC) HX-7 Covert
        "4E92B3C5-3358-44AA-8A87-F7F349F46F44", // ICA Tactical Shotgun (Wolverine) NPC
        "3804E585-8523-4F29-8404-B3A9A2F2E301", // Keycard Hacker Filur
        "785C3C6B-1272-4853-94F0-A41D52F64795", // NPC Hunting Shotgun

        // Medicine, poison, and cure items
        "D0438077-24EB-4C1D-A0F3-ABD53D18FC27", // (Dartgun) Cure I
        "C2B2A4F4-2B69-49BA-803D-C4A1A14694FD", // (Dartgun) Cure II
        "6B33CE1C-B7D8-4A4D-998F-6ABDA36100B0", // (Device) Device Cure Gas Remote Audio Distraction
        "B128A36E-D64D-4864-97E3-325A616126D8", // (Device) Remote Cure Gas Large
        "9533E414-59AF-4C58-8708-8C6B1E5A2A8E", // (Device) Remote Cure Gas Small
        "2AF583DC-C85D-45AD-B1BB-67DC5E6E4573", // (Sheep UNIQUE) Poisonable Cup of Tea
        "6C3854F6-DBE0-410C-BD01-DDC35B402D0C", // (Tool) Coin Cure
        "49C5EDFA-BD1C-4914-8368-BAC3B65BECF1", // Bergs Drugs
        "E45C295D-60DD-4CBA-A01B-0DC1B6F1B17C", // Experimental medicine
        "A1F89118-D345-4367-9423-620C3EF5DFBA", // Pill Glass (Lethal Poison)
        "8EE26350-67F9-48BD-983E-8F276EEA04CC", // Poison Emetic Mushroom
        "8ACF667C-071B-4A82-B58C-B1B95C5B71E8", // Poison_Pills_Baiju_ParalyticAntidote
        "D5CBB933-770C-495C-8C07-EECA9945D64E", // Poison_Vial_Baiju_Paralytic
        "BB6FEE5E-8B7A-445A-9E68-9168CE9DB66A", // Poisonable Cup of Tea (turned)
        "F174EC0E-DA7A-46BE-920A-782ECAF65CBC", // Poisonable_FoodPlate
        "22AE0D09-EC33-44A9-934D-E5AB43913B46", // Poisonable_FoodPlate_Asado
        "5FED2BB2-4FE9-4613-9F21-FEDC19BA5EB7", // Sedative Poison ReTYNZINE

        // Explosives and tactical gadgets
        "FF72222F-FD36-426C-B6F0-883CC813E3EE", // archteype c4 remote explosive
        "EE25FC91-E42E-4044-99B4-B3C4206D250D", // Exploding Watch Battery
        "B75010E8-ED6E-4646-BA22-73A30A7A990D", // Explosive Snow Globe
        "59B5731D-2DE8-4175-9BE0-92FBC2C3E603", // Gadget Wristwatch Alarm
        "9F63CD09-94F7-483B-93D9-4FFED6A0B331", // ICA Device Modular Remote Explosive Wolverine
        "30AE88F2-9F08-442A-A19A-64540D67EC93", // Mine Disarm Device
        "9EAB85B0-D0DB-4867-AD02-32DF253D3F5F", // RemoteExplosiveDuck Bundle

        // Utility items, keys, phones, cameras, and tools
        "D2C67AD3-95EB-4A21-A9B1-0C986AB4C091", // Burner  Phone Social
        "7CFD02AB-C43F-4DAD-9D7E-45D132B8818D", // Burner Phone Business
        "9F98BAFE-E3E0-4753-96A1-D77BAB212B94", // Burner Phone Courier
        "F3F8AC31-195D-4701-9E7C-775D621A405A", // Gadget_Camera
        "DE11AAC7-BECE-4BC8-BFD4-858EE178B625", // Gadget_Camera_Tagging
        "27F09DF0-93AC-4222-A546-F60D7AF8B5F6", // Key (Cultist Apartment)
        "606A9606-8C05-4DCD-93FA-EC9CDC13F357", // Pneumatic Wrench
        "ECF022DB-ECFD-48C0-97B5-2258E4E89A65", // Screwdriver Rusty One Time Use
        "6294F1C4-68DB-477F-B7C9-8C9825C077A1", // Tool_Rusty_Metal_Rod_OneTimeUse
        "14BF3A3C-54C7-4C98-9452-1E7F67DA09BF", // Tool_Rusty_Nail_OneTimeUse
        "46F0749F-8775-447C-BCAC-45E2E40A505D", // Tool_Rusty_Nail_OneTimeUse_EG
        "0576A20C-581B-4705-8B9D-464E077D117E", // WetFloorSign

        // Food, drink, bottles, glassware, and tableware
        "1A78F9C3-F0C1-493F-84C7-185B462B7EAB", // (Gadget) Power Bait (fish bait)
        "9BF224C4-6F9B-45F3-AD1E-2A3D9C12AFDD", // (Melee) Honey jar
        "0FF22CF7-A472-48D6-87EB-1B307BC5C576", // Apple
        "F8A13861-A166-4C81-AC5D-6D4E82A8307C", // Apple T
        "E755471F-E6FD-438F-B343-7C98FBB50107", // Apricot
        "903D273C-C750-441D-916A-31557FEA3382", // Banana
        "812BD23B-4998-4FB8-8625-1B7BADEC2746", // Bottle
        "25BC1A6D-C618-43EE-9C1F-81347ED430A6", // Burger
        "9526229D-B205-42EE-A754-C135ADF09750", // Champagne
        "6F750287-098A-4922-A254-23E1467E56A6", // Champagne Glass 120
        "3920B0CF-035A-481B-A9F6-E100676BEBDD", // Cup
        "4F2F4E60-D7F0-407B-B624-DE880F9E3015", // Cup
        "80823A7E-274E-42D9-AE44-816D9ABA4D31", // Cup_Tea
        "C9FF0E4E-844F-4B38-9582-80A1D8603EF6", // Empty Glass (Red)
        "4D0D6B2A-DD81-474C-A412-3BF19AF8233D", // Fish
        "1F11F901-2DBE-4E48-A77A-74C110B93DA0", // Fish_B
        "1374A658-91D2-4ABD-B339-28C7835467E8", // Glass (Red)
        "947B96D5-4BE2-4805-855C-4D9B5A53A440", // Glass (Red) (FreeAlign)
        "367AB7D6-31C5-4255-ACE0-574E72F8471D", // Glass (Red) (FreeAlign) (Fillable)
        "FCF98451-055A-46B0-949A-06A6F2B5875D", // Glass (Red) (SniperOnly)
        "633BAFBA-A05E-492C-B103-4EFD5C838DF9", // Glass (Red) 120
        "CCE0BCE9-52AD-47C3-BEDC-10B522693B38", // Glass (Tiki)
        "FCDD13CE-7385-40CB-B21C-86438B662096", // Glass (Tiki)
        "90F54148-45DD-4959-9D80-7764B452E85E", // Glass (White)
        "34D7BFBE-18A7-47F8-89E2-44F23F1482DC", // Glass of Champagne
        "51F592F4-A9A6-47A7-8143-113BC57EBF45", // Holipot
        "7B25EBD9-3993-4BE5-88D4-E9B308498BE7", // Jar Bellini A
        "5C7474AD-D245-43CA-948D-8800B2324E8C", // Lethal fugu fish
        "CDFE7F81-A275-4279-B126-71F93DDA32BF", // Melee Bloody Cocaine Brick
        "C6E9414E-E2CE-470A-95BD-14CD25225878", // Melee Cocaine Brick
        "0B97C464-7DF0-421E-844F-DF08AD563D7F", // Melee Waterbottle
        "3A359494-EE05-4FEA-BEAC-8726233A55BF", // Melee WhiskyBottle
        "EADAEFFD-C523-4942-B3E8-C475B68CC818", // Melee WineBottle
        "7F89BF01-235B-4AC5-9154-3584BA311E24", // Melee_CognacBottle
        "22B26943-65DD-4FDC-885A-CBC20A999C9A", // Melee_Fish_Fugu
        "3F9ED406-8DE0-4466-B393-38A7F905D859", // Melee_Llama_WineBottle_BlackLabel_Malbec
        "40766E9D-EB46-474E-B5CE-927E3E70F0C6", // Melee_Llama_WineBottle_BlueLabel_PinotNoir
        "299EAE90-4744-4557-B30B-71382CBA2839", // Melee_Llama_WineBottle_RedLabel_Cabernet
        "2D960BF0-217C-400D-A1EE-F721E18F2926", // Melee_Llama_WineBottle_WhiteLabel_Vintage
        "A729D2C1-D89C-4DA8-A628-816BA87DC349", // Plastic Cup
        "CFD359E5-BC76-4975-8DB6-0462BC746407", // Plate
        "32243E45-9D17-412E-9B6A-53DF25B3ADD3", // Pretzel
        "6A1BA1BF-8A02-4C08-84C9-4DE26BC54944", // Red wine
        "8F6D512C-6C20-42C9-A58B-B7405247D46C", // Shaker
        "2450176D-7D2C-4E57-876A-78F4186E67A7", // SniperOnly_Bottle
        "004ECAC9-6AEE-4B30-A073-4399A94535D8", // Soda can
        "DE69CE1E-A24D-4ACC-895F-4C3A71F47BA8", // Soda can japanese
        "4FBE2D58-5088-4155-805F-FE4C5789B8BB", // Soda can jester
        "AFD1F201-D2A5-4D40-80B1-D81B0D9D2541", // Soda can jitter
        "C19F796E-E23F-4429-A046-47ED3D324359", // Soda can thwack
        "45C0CCFE-1AC3-4747-9571-FE7588FE6971", // Sugar
        "89785A8E-B2EB-45AA-961C-DBFBFEFED962", // White wine
        "3B65524A-0B68-4D17-841B-221C1E637560", // Wine Glass

        // Decorative props and valuables
        "97D74FA2-4832-4186-A447-C4B2E37D537A", // Bust
        "A6BCAC8B-9772-424E-B2C4-3BDB4DA0E349", // Bust
        "4B0DEF3B-7378-494D-B885-92C334F2F8CB", // Bust_Colombi
        "31F36818-623F-4C92-892F-D7B19BB325E1", // Bust_Hokkaido
        "B86B9ECE-C929-44F6-8903-8F2C817E2A19", // Cannon ball
        "0F901C2C-3BCC-42F8-ABC0-1F9B81FCD72F", // CigarBox
        "A42123A6-75EA-4687-96CF-B099A49D3529", // Crystal Ball
        "F1A463F7-3834-4506-8192-28DA9749A219", // Electronics Crate
        "8E5E14E9-3A6A-4D8A-8382-BC791F2B1211", // Football_Shoe
        "0A5BEBC8-0148-4745-90B2-F54B3C71116C", // Napoleon figure
        "A96CDBD8-9657-416A-87BF-D2ED21840794", // News Paper Roll
        "1ADDC0C6-68BD-4C29-972C-DA404BEA3715", // One_Hundred_Thousand_Euro_Poker_Chip
        "CB42999A-A4D6-456B-A457-33816DBCA4FD", // One_Million_Euro_Poker_Chip
        "BAD168BB-3629-42B3-BC57-604B03A81D30", // Package
        "A2FCE6CB-7B4A-4D2E-81B7-919BF7C5B7AD", // Pearl
        "95D1C5BD-72DE-4236-97C0-B96FC5D92FA8", // Pool ball
        "6956A228-A1FF-4875-9FB9-F75DF316EF99", // Safehouse Blueprints
        "706CB615-E66D-49F3-86BB-899FA7117BCF", // Skyscraper Model
        "0FAE61C5-6410-417F-BDBE-B9E3605B53DA", // Styrofoam Box
        "BB42BBB6-289B-46F6-83AE-201385D0DEB1", // Styrofoam Crate
        "AC83255F-4419-4BCA-A016-52F1326B57C5", // VHS

        // Melee clutter, seasonal, and event props
        "75A0D0DE-FE3C-47D3-B64F-7FC446EE59C4", // (Fiberwire) Quickdraw
        "7BC45270-83FE-4CF6-AD10-7D1B0CF3A3FD", // (Melee) Seashell
        "4FAD7437-59E9-4CA9-9B31-A6D97484216B", // (Melee) Small Violin
        "A6706101-3AAF-4797-A0F8-A5B6AAC9CDFE", // (Melee) Snow Ball
        "79F8C0E9-4690-4EBF-B2B3-FD8411A1407F", // (S3) SMG_DAK_X2_RUSTY
        "5CC4D1EA-B4FA-4667-BA3A-B6E859F03059", // Brick
        "4EEDE7EE-582B-49A4-B438-2418D82671D9", // Bulldog Cane
        "E51BC878-281A-4711-B8A6-3088EBD1A27E", // Butchers Saw
        "D2F4E54F-1EB8-482D-9732-A9159B1A9229", // Colombia Melee Cocaine Bus
        "5FFDA47B-F445-44C7-8812-8D7E39E4928C", // Crowbar Holiday Present
        "6B87C27D-0D73-4C63-B852-5A9C7A9FFB90", // (Melee) Feather Duster
        "2ADD9602-CDA7-43FD-9758-6269C8FBB233", // Fire Axe Holiday Present
        "987D9C9F-203D-44D9-BBF8-BF703F349565", // Firepoker
        "A1495DBF-4CEB-4109-85BD-0FA43BD654C8", // FirepokerHolidayPresent
        "8F0931B1-3C3A-40E8-941B-70E85C04E580", // Greed Cane
        "5DB9CEFD-391E-4C35-A4C4-BB672AC9B996", // Machete_Jungle
        "1973AE7E-538C-4A43-98AF-208B9893D246", // Maori Paddle A
        "74B04D1F-8AC9-46A0-9A6C-8579CF03276F", // Melee_Didgeridoo
        "1279B3DB-4AFC-4CEB-8B00-A3C2414B0B72", // Melee_Purple_EasterEgg
        "E98F44FD-7F36-46A8-AE3C-BF080E8454D3", // Melee_Umbrella
        "4EC12492-E385-424D-9F23-6F605501242D", // Melee_Yellow_EasterEgg
        "58DAD7EA-F047-469F-92A1-D915A462C14C", // Pumpkin (small)
        "81654161-7711-4985-8056-8651A381D3CA", // Rake
        "6696A10D-9138-4184-8104-A6C7EC2E0EB1", // Reward Hunting Shotgun Deluxe
        "FBA6E133-78D1-4AF1-8450-1FF30466C553", // RustyPirateSaber
        "CB34F363-3534-46FF-B036-D49F1329F300", // Torch
        "0705964D-DAB5-45B6-96AE-30CD4C2F0DEC", // Wooden Paddle
    };

    bool IsBlacklistedRepositoryId(std::string p_Id)
    {
        std::transform(p_Id.begin(), p_Id.end(), p_Id.begin(), ::toupper);
        return std::find(
            s_BlacklistedRepositoryIds.begin(),
            s_BlacklistedRepositoryIds.end(),
            p_Id
        ) != s_BlacklistedRepositoryIds.end();
    }
}



RandomItems::RandomItems()
    : m_RandomGenerator(std::mt19937{ std::random_device{}() }),
      m_Distribution(0, 0),  // temporary; we will reset range after loading items
      m_CategoryEnabled(m_AllCategories.size(), true)
{
}


/**
 * Called when the game engine has finished initializing.
 * Registers the frame update delegate for continuous updates.
 */
void RandomItems::OnEngineInitialized() {
    Logger::Info("RandomItems has been initialized!");

    const ZMemberDelegate<RandomItems, void(const SGameUpdateEvent&)> s_Delegate(this, &RandomItems::OnFrameUpdate);
    Globals::GameLoopManager->RegisterFrameUpdate(s_Delegate, 1, EUpdateMode::eUpdatePlayMode);
}




/**
 * Destructor. Unregisters the frame update delegate to clean up resources.
 */
RandomItems::~RandomItems() {
    const ZMemberDelegate<RandomItems, void(const SGameUpdateEvent&)> s_Delegate(this, &RandomItems::OnFrameUpdate);
    Globals::GameLoopManager->UnregisterFrameUpdate(s_Delegate, 1, EUpdateMode::eUpdatePlayMode);
}

/**
 * Draws the toggle button in the main mod menu.
 */
void RandomItems::OnDrawMenu() {
    if (ImGui::Button(ICON_MD_LOCAL_FIRE_DEPARTMENT " Random Items")) {
        m_ShowMessage = !m_ShowMessage;
    }
}

/**
 * Renders the mod UI, including controls for start/stop, delay, spawn mode, and filters.
 * @param p_HasFocus Whether the UI window currently has input focus.
 */
void RandomItems::OnDrawUI(bool p_HasFocus) {
    if (m_ShowMessage && p_HasFocus) {
        if (ImGui::Begin(ICON_MD_LOCAL_FIRE_DEPARTMENT " Random Items", &m_ShowMessage)) {
            if (/*m_SpawnInWorld*/ false) {
                ImGui::SetWindowSize(ImVec2(611, 359));
            }
            else {
                ImGui::SetWindowSize(ImVec2(427, 300));
            }

            if (ImGui::Button(m_Running ? "Stop" : "Start")) {
                if (!m_Running) LoadRepositoryProps();
                m_Running = !m_Running;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Starting for the first time might freeze the game for a few seconds.");
            }
            ImGui::InputDouble("Delay (in s)", &m_DelaySeconds);
            ImGui::Checkbox("Spawn in world", &m_SpawnInWorld);
            ImGui::Checkbox("AI Can See Items", &m_AICanSeeItems);
            /*if (m_SpawnInWorld) {
                ImGui::InputFloat3("Item position", m_HitmanItemPosition);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("The position where the item will be spawned relative to the player position.");
                }
            }*/
            ImGui::SeparatorText("Experimental");
            // ────────────── Category Filter Menu ──────────────
            if (ImGui::CollapsingHeader("Category Filters")) {
                ImGui::TextWrapped("Toggle which categories to include when rebuilding pool:");
                for (size_t i = 0; i < m_AllCategories.size(); ++i) {
                    // pull out a real bool from the vector<bool> proxy
                    bool enabled = m_CategoryEnabled[i];

                    // render the checkbox; ImGui will modify 'enabled' if clicked
                    if (ImGui::Checkbox(m_AllCategories[i].c_str(), &enabled)) {
                        // write it back into your vector<bool>
                        m_CategoryEnabled[i] = enabled;
                    }
                }
            }


            ImGui::Checkbox("Include items without title", &m_IncludeItemsWithoutTitle);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("This will include more items, increasing the time to build the item pool and including some buggy items that can't actually spawn.");
            }
            if (ImGui::Button("Rebuild Item Pool")) {
                LoadRepositoryProps();
            }
        }
        ImGui::End();
    }
}

/**
 * Called every frame when the game is updating. Accumulates time and spawns items at intervals.
 * @param p_UpdateEvent Contains timing information for this frame.
 */
void RandomItems::OnFrameUpdate(const SGameUpdateEvent& p_UpdateEvent) {
    if (!m_Running) return;

    m_ElapsedTime += p_UpdateEvent.m_GameTimeDelta.ToSeconds();
    if (m_ElapsedTime >= m_DelaySeconds)
    {
        GiveRandomItem();
        m_ElapsedTime = 0.0;
    }
}

/**
 * Retrieves the repository pair (title and ID) at the given index in the map.
 * @param s_Index Zero-based index into the repository properties map.
 * @return Pair of item title string and repository ID.
 * @throws std::out_of_range if index is invalid.
 */


/**
 * Loads and filters the repository of available items from the game resource.
 * Populates m_RepositoryProps based on category and title inclusion settings.
 */
void RandomItems::LoadRepositoryProps()
{
    Logger::Info("Loading repository (your game will freeze shortly)");

    // 1) Clear out any old entries
    m_RepositoryProps.clear();

    // 2) Build a list of only the categories the user has checked
    std::vector<std::string> s_IncludedCategories;
    for (size_t i = 0; i < m_AllCategories.size(); ++i) {
        if (m_CategoryEnabled[i]) {
            s_IncludedCategories.push_back(m_AllCategories[i]);
        }
    }

    // 3) Ensure the repository resource is loaded
    if (!m_RepositoryResource)
    {
        const auto s_ID = ResId<"[assembly:/repository/pro.repo].pc_repo">;
        Globals::ResourceManager->GetResourcePtr(m_RepositoryResource, s_ID, 0);
    }

    // 4) Only proceed if we have valid data
    if (m_RepositoryResource.GetResourceInfo().status == RESOURCE_STATUS_VALID)
    {
        // Raw map: ZRepositoryID → ZDynamicObject
        auto* s_RepositoryData = static_cast<
            THashMap<ZRepositoryID, ZDynamicObject, TDefaultHashMapPolicy<ZRepositoryID>>*
        >(m_RepositoryResource.GetResourceData());

        // 5) Iterate every entry in the repo
        for (auto it = s_RepositoryData->begin(); it != s_RepositoryData->end(); ++it)
        {
            const ZDynamicObject* s_DynamicObject = &it->second;
            const auto* s_Entries = s_DynamicObject->As<TArray<SDynamicObjectKeyValuePair>>();

            std::string s_Id;
            bool s_HasTitle = false;
            bool s_Included = true;
            std::string s_TitleToAdd;
            ZRepositoryID s_RepoIdToAdd("");

            // 6) Pull out each field we care about
            for (const auto& kv : *s_Entries)
            {
                std::string s_Key = kv.sKey.c_str();

                if (s_Key == "ID_")
                {
                    s_Id = ConvertDynamicObjectValueTString(kv.value);
                }
                else if (s_Key == "Title")
                {
                    s_HasTitle    = true;
                    std::string t = ConvertDynamicObjectValueTString(kv.value);
                    s_TitleToAdd  = t;
                    s_RepoIdToAdd = ZRepositoryID(ZString(s_Id));

                    // filter out blank titles if the user disabled them
                    if (t.empty() && !m_IncludeItemsWithoutTitle)
                        s_Included = false;
                }
                else if (s_Key == "InventoryCategoryIcon")
                {
                    // uppercase the category
                    std::string cat = ConvertDynamicObjectValueTString(kv.value);
                    std::transform(cat.begin(), cat.end(), cat.begin(), ::toupper);

                    // check it against our dynamic list
                    bool match = false;
                    for (auto& want : s_IncludedCategories)
                    {
                        std::string tmp = want;
                        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
                        if (tmp == cat) { match = true; break; }
                    }
                    if (!match)
                        s_Included = false;
                }
                else if (s_Key == "IsHitmanSuit")
                {
                    // never spawn suits
                    s_Included = false;
                    break;
                }
                // else {Logger::Debug("Unresolved skey: {}", s_Key);}
            }

            // 7) If it passed all filters, add it to the pool
            if (s_Included && IsBlacklistedRepositoryId(s_Id))
            {
                s_Included = false;
            }

            if (s_Included && (s_HasTitle || m_IncludeItemsWithoutTitle))
            {
                m_RepositoryProps.push_back({ s_TitleToAdd, s_RepoIdToAdd });
            }
        }
    }

    if (!m_RepositoryProps.empty()) {
        m_Distribution = std::uniform_int_distribution<size_t>(
            0, m_RepositoryProps.size() - 1
        );
    }
}


/**
 * Converts a dynamic object value to its string representation.
 * Supports ZString, bool, and float64 types; others return the type name.
 * @param p_DynamicObject The dynamic object to convert.
 * @return Value as a UTF-8 encoded std::string.
 */
std::string RandomItems::ConvertDynamicObjectValueTString(const ZDynamicObject& p_DynamicObject)
{
    // In our usage, all keys we care about (ID_, Title, InventoryCategoryIcon)
    // are ZString values, so we can simply treat them as such.
    const auto valuePtr = p_DynamicObject.As<ZString>();
    if (!valuePtr) {
        return {};
    }

    return valuePtr->c_str();
}



/**
 * Chooses a random item from the repository pool and either spawns it in the world or adds it to inventory.
 */
void RandomItems::GiveRandomItem()
{
    if (m_RepositoryProps.size() == 0)
    {
        Logger::Info("loading repository props (your game might freeze shortly)");
        LoadRepositoryProps();
    }
    if (m_RepositoryProps.empty())
    {
        Logger::Error("Repository props are still empty after loading; cannot pick random item.");
        return;
    }

    const size_t s_RandomIndex = m_Distribution(m_RandomGenerator);
    const auto& s_PropPair = m_RepositoryProps[s_RandomIndex];

    auto s_LocalHitman = SDK()->GetLocalPlayer();
    if (!s_LocalHitman) {
        Logger::Error("No local hitman.");
        return;
    }

    Logger::Info("Spawning: {},  {}", s_PropPair.first, s_PropPair.second.ToString());
    if(m_SpawnInWorld) {
        // Logger::Info("Spawning in world: {}", s_PropPair.first);
        ZSpatialEntity* s_HitmanSpatial = s_LocalHitman.m_entityRef.QueryInterface<ZSpatialEntity>();

        const auto s_Scene = Globals::Hitman5Module->m_pEntitySceneContext->m_pScene;
        if (!s_Scene) {
            Logger::Warn("no scene loaded");
            return;
        }

        const auto s_ItemSpawnerID = ResId<"[modules:/zitemspawner.class].pc_entitytype">;
        const auto s_ItemRepoKeyID = ResId<"[modules:/zitemrepositorykeyentity.class].pc_entitytype">;

        TResourcePtr<ZTemplateEntityFactory> s_Resource, s_Resource2;

        Globals::ResourceManager->GetResourcePtr(s_Resource, s_ItemSpawnerID, 0);
        Globals::ResourceManager->GetResourcePtr(s_Resource2, s_ItemRepoKeyID, 0);

        if (!s_Resource)
        {
            Logger::Error("resource not loaded");
            return;
        }

        ZEntityRef s_ItemSpawnerEntity, s_ItemRepoKey;
        SExternalReferences s_EmptyRefs{};
        Functions::ZEntityManager_NewEntity->Call(
    Globals::EntityManager,
    s_ItemSpawnerEntity,
    ZString(""),
    s_Resource,
    s_Scene.m_entityRef,
    s_EmptyRefs,
    static_cast<uint64_t>(-1)
);

        Functions::ZEntityManager_NewEntity->Call(
            Globals::EntityManager,
            s_ItemRepoKey,
            ZString(""),
            s_Resource2,
            s_Scene.m_entityRef,
            s_EmptyRefs,
            static_cast<uint64_t>(-1)
        );


        if (!s_ItemSpawnerEntity)
        {
            Logger::Error("failed to spawn item spawner");
            return;
        }

        if (!s_ItemRepoKey)
        {
            Logger::Error("failed to spawn item repo key entity");
            return;
        }

        const auto s_ItemSpawner = s_ItemSpawnerEntity.QueryInterface<ZItemSpawner>();

        s_ItemSpawner->m_ePhysicsMode = m_AICanSeeItems
            ? ZItemSpawner::EPhysicsMode::EPM_DYNAMIC
            : ZItemSpawner::EPhysicsMode::EPM_KINEMATIC;
        s_ItemSpawner->m_bSetAIPerceptible = m_AICanSeeItems;
        s_ItemSpawner->m_rMainItemKey.m_entityRef = s_ItemRepoKey;
        s_ItemSpawner->m_rMainItemKey.m_pInterfaceRef = s_ItemRepoKey.QueryInterface<ZItemRepositoryKeyEntity>();
        s_ItemSpawner->m_rMainItemKey.m_pInterfaceRef->m_RepositoryId = s_PropPair.second;
        s_ItemSpawner->m_bUsePlacementAttach = false;
        s_ItemSpawner->m_eDisposalTypeOverwrite = EDisposalType::DISPOSAL_DESTROY;
        s_ItemSpawner->SetObjectToWorldMatrixFromEditor(s_HitmanSpatial->GetObjectToWorldMatrix());

        Functions::ZItemSpawner_RequestContentLoad->Call(s_ItemSpawner);
    }  else {
        const TArray<TEntityRef<ZCharacterSubcontroller>>* s_Controllers = &s_LocalHitman.m_pInterfaceRef->m_pCharacter.m_pInterfaceRef->m_rSubcontrollerContainer.m_pInterfaceRef->m_aReferencedControllers;
        auto* s_Inventory = static_cast<ZCharacterSubcontrollerInventory*>(s_Controllers->operator[](6).m_pInterfaceRef);

        TArray<ZRepositoryID> s_ModifierIds;

        Functions::ZCharacterSubcontrollerInventory_CreateItem->Call(
            s_Inventory,
            s_PropPair.second,                // repId
            ZString(""),                      // sOnlineInstanceId (we leave it empty)
            s_ModifierIds,                    // const TArray<ZRepositoryID>&
            ZCharacterSubcontrollerInventory::ECreateItemType::ECIT_CarriedItem
        );

    }
}

// Macro to register plugin with the Hitman mod framework
DECLARE_ZHM_PLUGIN(RandomItems);
